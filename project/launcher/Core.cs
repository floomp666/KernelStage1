using System;
using System.IO;
using System.Text;
using System.Diagnostics;
using System.ComponentModel;
using System.Runtime.InteropServices;
using Microsoft.Win32.SafeHandles;
namespace ProjectLauncher {
public static class Core {
    [DllImport("kernel32", SetLastError=true)] static extern IntPtr OpenProcess(uint access, bool inherit, int pid);
    [DllImport("kernel32")] static extern bool CloseHandle(IntPtr h);
    [DllImport("kernel32", SetLastError=true)] static extern bool GetProcessTimes(IntPtr h, out long creation, out long exit, out long kernel, out long user);
    [DllImport("kernel32", SetLastError=true)] static extern bool IsWow64Process(IntPtr h, out bool wow);
    [DllImport("kernel32", SetLastError=true)] static extern IntPtr VirtualAllocEx(IntPtr h, IntPtr address, UIntPtr size, uint allocation, uint protection);
    [DllImport("kernel32", SetLastError=true)] static extern bool VirtualFreeEx(IntPtr h, IntPtr address, UIntPtr size, uint type);
    [DllImport("kernel32", SetLastError=true)] static extern bool WriteProcessMemory(IntPtr h, IntPtr address, byte[] data, UIntPtr size, out UIntPtr written);
    [DllImport("kernel32", SetLastError=true)] static extern IntPtr CreateRemoteThread(IntPtr h, IntPtr attr, UIntPtr stack, IntPtr start, IntPtr parameter, uint flags, IntPtr id);
    [DllImport("kernel32", SetLastError=true)] static extern uint WaitForSingleObject(IntPtr h, uint timeout);
    [DllImport("kernel32", CharSet=CharSet.Unicode)] static extern IntPtr GetModuleHandle(string name);
    [DllImport("kernel32", CharSet=CharSet.Ansi, ExactSpelling=true, SetLastError=true)] static extern IntPtr GetProcAddress(IntPtr module, string name);
    [DllImport("kernel32", CharSet=CharSet.Unicode, SetLastError=true)] static extern bool GetModuleHandleEx(uint flags, IntPtr address, out IntPtr module);
    [DllImport("kernel32", CharSet=CharSet.Unicode)] static extern uint GetModuleFileName(IntPtr module, StringBuilder name, int size);
    [DllImport("kernel32", CharSet=CharSet.Unicode, SetLastError=true)] static extern SafeFileHandle CreateFile(string path,uint access,uint share,IntPtr security,uint creation,uint attributes,IntPtr template);
    [DllImport("kernel32", SetLastError=true)] static extern bool DeviceIoControl(SafeFileHandle h,uint code,IntPtr input,int inputSize,[Out] uint[] output,int outputSize,out int returned,IntPtr overlapped);
    public static Exception Error(string step) { return new Win32Exception(Marshal.GetLastWin32Error(),step); }
    public static string ValidateDll(string file) {
        file=Path.GetFullPath(file);
        using(var f=File.OpenRead(file)) using(var b=new BinaryReader(f)) {
            if(f.Length<64 || b.ReadUInt16()!=0x5a4d) throw new InvalidDataException("Not a PE file");
            f.Position=60; int pe=b.ReadInt32();
            if(pe<64 || pe>f.Length-26) throw new InvalidDataException("Invalid PE offset");
            f.Position=pe;
            if(b.ReadUInt32()!=0x4550 || b.ReadUInt16()!=0x8664) throw new InvalidDataException("An x64 DLL is required");
            f.Position=pe+22; if((b.ReadUInt16()&0x2000)==0 || b.ReadUInt16()!=0x20b) throw new InvalidDataException("Not a PE32+ DLL");
        }
        return file;
    }
    public static void VerifyBridge() {
        using(var h=CreateFile(@"\\.\RustInternalBridge",0x80000000,0,IntPtr.Zero,3,0,IntPtr.Zero)) {
            if(h.IsInvalid) throw Error("Open driver bridge");
            uint[] output=new uint[2]; int returned;
            if(!DeviceIoControl(h,0x226000,IntPtr.Zero,0,output,8,out returned,IntPtr.Zero)) throw Error("Query driver ABI");
            if(returned!=8 || output[0]!=8 || output[1]!=1) throw new InvalidDataException("Driver ABI mismatch");
        }
    }
    static bool Loaded(int pid,string path) {
        using(var process=Process.GetProcessById(pid)) foreach(ProcessModule module in process.Modules)
            if(String.Equals(Path.GetFullPath(module.FileName),path,StringComparison.OrdinalIgnoreCase)) return true;
        return false;
    }
    public static string LoadDll(int pid, long expectedStart, string path) {
        path=ValidateDll(path);
        if(IntPtr.Size!=8) throw new InvalidOperationException("Use the x64 launcher");
        using(var target=Process.GetProcessById(pid)) {
            if(target.StartTime.ToUniversalTime().Ticks!=expectedStart) throw new InvalidOperationException("PID was reused; refresh the process list");
            IntPtr h=OpenProcess(0x0002|0x0400|0x0008|0x0020|0x0010|0x100000,false,pid);
            if(h==IntPtr.Zero) throw Error("Open selected process");
            IntPtr remote=IntPtr.Zero,thread=IntPtr.Zero; bool release=true;
            try {
                long created, exited, kernelTime, userTime;
                if(!GetProcessTimes(h,out created,out exited,out kernelTime,out userTime)) throw Error("Check selected process identity");
                if(DateTime.FromFileTimeUtc(created).Ticks!=expectedStart) throw new InvalidOperationException("Process identity changed; refresh selection");
                bool wow;
                if(!IsWow64Process(h,out wow)) throw Error("Check process architecture");
                if(wow) throw new InvalidOperationException("Selected process is x86; an x64 process is required");
                if(Loaded(pid,path)) return "DLL already loaded; initialization was not repeated";
                IntPtr local=GetProcAddress(GetModuleHandle("kernel32.dll"),"LoadLibraryW"), owner;
                if(local==IntPtr.Zero || !GetModuleHandleEx(6,local,out owner)) throw Error("Resolve LoadLibraryW");
                var name=new StringBuilder(32768); if(GetModuleFileName(owner,name,name.Capacity)==0) throw Error("Resolve module name");
                IntPtr remoteStart=IntPtr.Zero;
                foreach(ProcessModule module in target.Modules)
                    if(String.Equals(Path.GetFileName(module.FileName),Path.GetFileName(name.ToString()),StringComparison.OrdinalIgnoreCase))
                        remoteStart=new IntPtr(module.BaseAddress.ToInt64()+local.ToInt64()-owner.ToInt64());
                if(remoteStart==IntPtr.Zero) throw new InvalidOperationException("Loader module is absent in selected process");
                byte[] payload=Encoding.Unicode.GetBytes(path+"\0");
                remote=VirtualAllocEx(h,IntPtr.Zero,(UIntPtr)payload.Length,0x3000,4);
                if(remote==IntPtr.Zero) throw Error("Allocate DLL path");
                UIntPtr written;
                if(!WriteProcessMemory(h,remote,payload,(UIntPtr)payload.Length,out written) || written.ToUInt64()!=(ulong)payload.Length) throw Error("Write DLL path");
                thread=CreateRemoteThread(h,IntPtr.Zero,UIntPtr.Zero,remoteStart,remote,0,IntPtr.Zero);
                if(thread==IntPtr.Zero) throw Error("Start DLL loader");
                uint wait=WaitForSingleObject(thread,30000);
                if(wait!=0) { release=false; throw new TimeoutException("DLL loader did not finish. Its path buffer is retained; inspect target before retrying."); }
                if(!Loaded(pid,path)) throw new InvalidOperationException("DLL is not in module list. Check dependencies and DllMain result.");
                return "DLL mapped; DllMain completed. Asynchronous application initialization is not verified.";
            } finally {
                if(thread!=IntPtr.Zero) CloseHandle(thread);
                if(remote!=IntPtr.Zero && release) VirtualFreeEx(h,remote,UIntPtr.Zero,0x8000);
                CloseHandle(h);
            }
        }
    }
}
public sealed class DriverLease {
    const string Name="RustInternalBridge";
    bool started,created;
    [StructLayout(LayoutKind.Sequential)] struct Config { public uint type,start,error; public IntPtr binary,group; public uint tag; public IntPtr dependencies,account,display; }
    [StructLayout(LayoutKind.Sequential)] struct Status { public uint type,state,controls,win32,service,checkpoint,hint; }
    [DllImport("advapi32", CharSet=CharSet.Unicode,SetLastError=true)] static extern IntPtr OpenSCManager(string machine,string db,uint access);
    [DllImport("advapi32", CharSet=CharSet.Unicode,SetLastError=true)] static extern IntPtr OpenService(IntPtr manager,string name,uint access);
    [DllImport("advapi32", CharSet=CharSet.Unicode,SetLastError=true)] static extern IntPtr CreateService(IntPtr manager,string name,string display,uint access,uint type,uint start,uint error,string binary,string group,IntPtr tag,string dependencies,string account,string password);
    [DllImport("advapi32",CharSet=CharSet.Unicode,SetLastError=true)] static extern bool QueryServiceConfig(IntPtr service,IntPtr config,uint size,out uint needed);
    [DllImport("advapi32",SetLastError=true)] static extern bool StartService(IntPtr service,uint argc,IntPtr argv);
    [DllImport("advapi32",SetLastError=true)] static extern bool QueryServiceStatus(IntPtr service,out Status status);
    [DllImport("advapi32",SetLastError=true)] static extern bool ControlService(IntPtr service,uint control,out Status status);
    [DllImport("advapi32",SetLastError=true)] static extern bool DeleteService(IntPtr service);
    [DllImport("advapi32")] static extern bool CloseServiceHandle(IntPtr service);
    public void Start(string file) {
        file=Path.GetFullPath(file); if(!File.Exists(file)) throw new FileNotFoundException("Select a driver .sys",file);
        IntPtr scm=OpenSCManager(null,null,3); if(scm==IntPtr.Zero) throw Core.Error("Open SCM (administrator required)");
        IntPtr svc=IntPtr.Zero;
        try {
            const uint access=0x10|0x20|0x4|0x1|0x10000;
            svc=OpenService(scm,Name,access);
            if(svc==IntPtr.Zero) {
                if(Marshal.GetLastWin32Error()!=1060) throw Core.Error("Open driver service");
                svc=CreateService(scm,Name,Name,access,1,3,1,"\""+file+"\"",null,IntPtr.Zero,null,null,null);
                if(svc==IntPtr.Zero) throw Core.Error("Create driver service"); created=true;
            }
            uint needed; QueryServiceConfig(svc,IntPtr.Zero,0,out needed);
            if(needed==0) throw Core.Error("Read service configuration size");
            IntPtr buffer=Marshal.AllocHGlobal((int)needed);
            try {
                if(!QueryServiceConfig(svc,buffer,needed,out needed)) throw Core.Error("Read service configuration");
                var config=(Config)Marshal.PtrToStructure(buffer,typeof(Config));
                string existing=Environment.ExpandEnvironmentVariables(Marshal.PtrToStringUni(config.binary)).Trim('"');
                if(config.type!=1 || !String.Equals(Path.GetFullPath(existing),file,StringComparison.OrdinalIgnoreCase)) throw new InvalidOperationException("Existing RustInternalBridge service uses another file; configuration left unchanged.");
            } finally { Marshal.FreeHGlobal(buffer); }
            Status status; if(!QueryServiceStatus(svc,out status)) throw Core.Error("Query driver state");
            if(status.state!=4) {
                if(!StartService(svc,0,IntPtr.Zero)) {
                    int code=Marshal.GetLastWin32Error();
                    if(code!=1056) throw new Win32Exception(code,"Start driver; Windows signature policy applies");
                } else started=true;
            }
            Core.VerifyBridge();
        } catch {
            if(svc!=IntPtr.Zero) { Status st; if(started) ControlService(svc,1,out st); if(created) DeleteService(svc); }
            started=false; created=false; throw;
        } finally { if(svc!=IntPtr.Zero) CloseServiceHandle(svc); CloseServiceHandle(scm); }
    }
    public void Stop() {
        if(!started && !created) throw new InvalidOperationException("No driver service owned by this launcher session");
        IntPtr scm=OpenSCManager(null,null,1); if(scm==IntPtr.Zero) throw Core.Error("Open SCM");
        IntPtr svc=OpenService(scm,Name,0x20|0x4|0x10000);
        try {
            if(svc==IntPtr.Zero) throw Core.Error("Open driver service");
            Status st; if(started && !ControlService(svc,1,out st)) throw Core.Error("Stop driver");
            if(created && !DeleteService(svc)) throw Core.Error("Delete driver registration");
            started=false;created=false;
        } finally { if(svc!=IntPtr.Zero) CloseServiceHandle(svc); CloseServiceHandle(scm); }
    }
}
}
