#pragma once
class StyleManager {
public:
    void Styles() {
        auto& s = ImGui::GetStyle();
        s.WindowRounding=14; s.WindowPadding=ImVec2(0,0); s.WindowBorderSize=1;
        s.FrameRounding=6; s.FramePadding=ImVec2(10,8); s.FrameBorderSize=0;
        s.PopupRounding=8; s.PopupBorderSize=1; s.ChildRounding=9; s.ChildBorderSize=1;
        s.ItemSpacing=ImVec2(12,12); s.ItemInnerSpacing=ImVec2(8,6);
        s.ScrollbarRounding=8; s.ScrollbarSize=8; s.GrabRounding=4;
        s.WindowMinSize=ImVec2(320,240);
    }
    void Colors() {
        auto* c=ImGui::GetStyle().Colors;
        for(int i=0;i<ImGuiCol_COUNT;++i) c[i]=ImVec4(.12f,.16f,.20f,1);
        const ImVec4 accent(.36f,.85f,.72f,1), text(.89f,.93f,.95f,1), muted(.48f,.57f,.65f,1);
        c[ImGuiCol_WindowBg]=ImVec4(.04f,.052f,.068f,1);
        c[ImGuiCol_ChildBg]=ImVec4(.065f,.085f,.108f,1);
        c[ImGuiCol_PopupBg]=ImVec4(.08f,.10f,.13f,1);
        c[ImGuiCol_Text]=c[ImGuiCol_TextButton]=text; c[ImGuiCol_TextDisabled]=muted;
        c[ImGuiCol_TextHovered]=accent; c[ImGuiCol_Scheme]=accent;
        c[ImGuiCol_Border]=c[ImGuiCol_Separator]=ImVec4(.13f,.18f,.22f,1);
        c[ImGuiCol_BorderShadow]=ImVec4(0,0,0,0);
        c[ImGuiCol_FrameBg]=c[ImGuiCol_Button]=ImVec4(.09f,.12f,.15f,1);
        c[ImGuiCol_FrameBgHovered]=c[ImGuiCol_ButtonHovered]=ImVec4(.14f,.23f,.26f,1);
        c[ImGuiCol_FrameBgActive]=c[ImGuiCol_ButtonActive]=ImVec4(.17f,.33f,.32f,1);
        c[ImGuiCol_Header]=ImVec4(.13f,.28f,.27f,1);
        c[ImGuiCol_HeaderHovered]=ImVec4(.15f,.34f,.31f,1); c[ImGuiCol_HeaderActive]=ImVec4(.17f,.40f,.35f,1);
        c[ImGuiCol_CheckMark]=c[ImGuiCol_SliderGrab]=c[ImGuiCol_SliderGrabActive]=accent;
        c[ImGuiCol_TextSelectedBg]=ImVec4(.36f,.85f,.72f,.22f);
        c[ImGuiCol_ScrollbarBg]=ImVec4(0,0,0,0); c[ImGuiCol_ScrollbarGrab]=ImVec4(.22f,.30f,.34f,1);
        c[ImGuiCol_ScrollbarGrabHovered]=c[ImGuiCol_ScrollbarGrabActive]=accent;
        c[ImGuiCol_NavHighlight]=accent;
    }
    static StyleManager& get() { static StyleManager s; return s; }
};
