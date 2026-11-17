using System;
using System.IO;
using System.Diagnostics;
using System.Drawing;
using System.Windows.Forms;
using System.Threading;
namespace ProjectLauncher {
sealed class ProcessItem {
    public int Pid; public long Started; public string Name;
    public override string ToString() { return Name+"   [PID "+Pid+"]"; }
}
sealed class MainForm : Form {
    readonly TextBox dll=new TextBox(),sys=new TextBox(),log=new TextBox();
    readonly ListBox processes=new ListBox();
    readonly Button browseDll=new Button(),browseSys=new Button(),refresh=new Button(),launch=new Button(),stop=new Button();
    readonly DriverLease driver=new DriverLease();
    bool busy,driverReady;
    public MainForm() {
        Text="Project Launcher / x64"; ClientSize=new Size(760,690); FormBorderStyle=FormBorderStyle.FixedDialog; MaximizeBox=false;
        Font=new Font("Segoe UI",10); BackColor=Color.FromArgb(17,23,29); ForeColor=Color.Gainsboro;
        LabelAt("1. Выберите DLL (x64)",20,15); dll.SetBounds(20,42,590,28); dll.ReadOnly=true;
        browseDll.Text="DLL…"; browseDll.SetBounds(625,40,115,30); browseDll.Click+=delegate{Pick(dll,"DLL (*.dll)|*.dll");};
        LabelAt("2. Выберите процесс",20,90); processes.SetBounds(20,120,590,260);
        refresh.Text="Обновить"; refresh.SetBounds(625,120,115,32); refresh.Click+=delegate{RefreshProcesses();};
        LabelAt("3. Драйвер: запуск службы → проверка ABI → загрузка DLL",20,395); sys.SetBounds(20,423,590,28); sys.ReadOnly=true;
        string bundledDriver=Path.Combine(AppDomain.CurrentDomain.BaseDirectory,"RustInternalBridge.sys");
        sys.Text=File.Exists(bundledDriver) ? bundledDriver : Path.Combine(AppDomain.CurrentDomain.BaseDirectory,@"project\kernel\build\RustInternalBridge.sys");
        browseSys.Text="SYS…"; browseSys.SetBounds(625,421,115,30); browseSys.Click+=delegate{if(driverReady){Append("Сначала остановите драйвер этой сессии.");return;}Pick(sys,"Driver (*.sys)|*.sys");};
        launch.Text="Запустить"; launch.SetBounds(20,470,180,38); launch.BackColor=Color.FromArgb(45,110,95); launch.Click+=delegate{Launch();};
        stop.Text="Остановить свой драйвер"; stop.SetBounds(220,470,260,38); stop.Click+=delegate{Work(delegate{driver.Stop();driverReady=false;return "Драйвер остановлен; регистрация, созданная этой сессией, удалена. DLL остаётся в процессе.";});};
        log.Multiline=true;log.ReadOnly=true;log.ScrollBars=ScrollBars.Vertical;log.SetBounds(20,528,720,140);
        Controls.AddRange(new Control[]{dll,sys,processes,browseDll,browseSys,refresh,launch,stop,log});
        foreach(Control c in Controls) if(!(c is Label)) {c.ForeColor=Color.Gainsboro;if(c!=launch)c.BackColor=Color.FromArgb(30,40,49);}
        FormClosing+=delegate(object sender,FormClosingEventArgs e){if(busy){e.Cancel=true; MessageBox.Show("Дождитесь завершения текущего шага.");}};
        RefreshProcesses(); Append("Существующий драйвер не загружает DLL: загрузка использует Windows API. Смена процесса не делает DLL совместимой с ним.");
    }
    void LabelAt(string text,int x,int y){var l=new Label{Text=text,AutoSize=true};l.Location=new Point(x,y);Controls.Add(l);}
    void Pick(TextBox field,string filter){using(var d=new OpenFileDialog{Filter=filter,CheckFileExists=true,Multiselect=false})if(d.ShowDialog(this)==DialogResult.OK)field.Text=d.FileName;}
    void RefreshProcesses(){
        processes.Items.Clear();
        var items=new System.Collections.Generic.List<ProcessItem>();
        int self; using(var current=Process.GetCurrentProcess()) self=current.Id;
        foreach(var p in Process.GetProcesses()) using(p){try{if(p.Id>4 && p.Id!=self)items.Add(new ProcessItem{Pid=p.Id,Started=p.StartTime.ToUniversalTime().Ticks,Name=p.ProcessName});}catch(InvalidOperationException){}catch(System.ComponentModel.Win32Exception){}}
        items.Sort(delegate(ProcessItem a,ProcessItem b){return String.Compare(a.Name,b.Name,StringComparison.OrdinalIgnoreCase);});
        foreach(var item in items) processes.Items.Add(item);
    }
    void Append(string text){log.AppendText(DateTime.Now.ToString("HH:mm:ss")+"  "+text+Environment.NewLine);}
    void Work(Func<string> action){
        if(busy)return;busy=true;foreach(var c in new Control[]{launch,stop,refresh,browseDll,browseSys,processes})c.Enabled=false;
        ThreadPool.QueueUserWorkItem(delegate{string result;try{result=action();}catch(Exception e){result=e.GetType().Name+": "+e.Message;}BeginInvoke(new Action(delegate{Append(result);busy=false;foreach(var c in new Control[]{launch,stop,refresh,browseDll,browseSys,processes})c.Enabled=true;}));});
    }
    void Launch(){
        var item=processes.SelectedItem as ProcessItem;
        if(item==null){Append("Выберите процесс из списка.");return;}
        string selectedDll=dll.Text,selectedSys=sys.Text;
        if(MessageBox.Show(this,"Загрузить DLL в "+item.Name+" (PID "+item.Pid+")? Несовместимая DLL может завершить процесс. Драйвер останется запущенным до остановки.","Подтверждение",MessageBoxButtons.OKCancel,MessageBoxIcon.Warning)!=DialogResult.OK)return;
        Work(delegate{
            Core.ValidateDll(selectedDll);
            if(!driverReady){driver.Start(selectedSys);driverReady=true;} else Core.VerifyBridge();
            return Core.LoadDll(item.Pid,item.Started,selectedDll);
        });
    }
    [STAThread] static void Main(){Application.EnableVisualStyles();Application.SetCompatibleTextRenderingDefault(false);Application.Run(new MainForm());}
}
}
