#pragma once
#include <imgui.h>
#include <functional>
#include <vector>
#include <algorithm>
namespace interface_v2 {
struct Page { const char* label; std::vector<const char*> sections; };
struct Model {
    int page = 0, section = 0;
    char search[128]{};
    bool russian = false;
    bool clear_requested = false;
};
inline const char* tr(const Model& m, const char* en, const char* ru) { return m.russian ? ru : en; }
inline void draw(Model& m, const std::vector<Page>& pages, const std::function<void()>& content) {
    if (pages.empty()) return;
    m.page = (std::max)(0, (std::min)(m.page, int(pages.size()) - 1));
    const ImVec2 screen = ImGui::GetIO().DisplaySize;
    const ImVec2 dimensions((std::max)(400.f, (std::min)(1040.f, screen.x - 24.f)),
                            (std::max)(340.f, (std::min)(690.f, screen.y - 24.f)));
    ImGui::SetNextWindowSize(dimensions);
    ImGui::SetNextWindowPos(ImVec2((screen.x-dimensions.x)*0.5f, (screen.y-dimensions.y)*0.5f), ImGuiCond_FirstUseEver);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0,0));
    const bool visible = ImGui::Begin("##interface-v2", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
    ImGui::PopStyleVar();
    if (visible) {
        const float rail = dimensions.x < 760.f ? 145.f : 194.f;
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(18,22));
        ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(.055f,.071f,.090f,1));
        ImGui::BeginChild("navigation", ImVec2(rail,0), ImGuiChildFlags_AlwaysUseWindowPadding);
        ImGui::PopStyleColor(); ImGui::PopStyleVar();
        ImGui::TextColored(ImVec4(.36f,.85f,.72f,1), "R / INTERNAL");
        ImGui::TextDisabled("CONTROL CENTER");
        ImGui::Dummy(ImVec2(0,24));
        for (int i=0; i<int(pages.size()); ++i) {
            ImGui::PushID(i);
            if (ImGui::Selectable(pages[i].label, m.page==i, 0, ImVec2(0,40))) {
                if (m.page != i) m.section=0;
                m.page=i; m.search[0]=0; m.clear_requested=true;
            }
            ImGui::PopID();
        }
        ImGui::Dummy(ImVec2(0,18)); ImGui::Separator();
        ImGui::TextDisabled(tr(m,"Language",u8"Язык"));
        if (ImGui::Button(m.russian ? "RU / English" : "EN / Russian", ImVec2(-1,32))) m.russian=!m.russian;
        ImGui::TextDisabled(tr(m,"INSERT - menu",u8"INSERT - меню"));
        ImGui::EndChild();
        ImGui::SameLine(0,0);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(24,22));
        ImGui::BeginChild("workspace", ImVec2(0,0), ImGuiChildFlags_AlwaysUseWindowPadding);
        ImGui::PopStyleVar();
        ImGui::TextDisabled(tr(m,"WORKSPACE / SETTINGS",u8"ПАНЕЛЬ / НАСТРОЙКИ"));
        ImGui::SetWindowFontScale(1.45f); ImGui::TextUnformatted(pages[m.page].label); ImGui::SetWindowFontScale(1.f);
        ImGui::Dummy(ImVec2(0,6));
        const float available = ImGui::GetContentRegionAvail().x;
        ImGui::SetNextItemWidth((std::max)(80.f,available-78.f));
        ImGui::InputTextWithHint("##global-search",tr(m,"Search settings...",u8"Поиск настроек..."),m.search,sizeof(m.search));
        ImGui::SameLine();
        if (ImGui::Button(tr(m,"Clear",u8"Сброс"), ImVec2(68,0))) { m.search[0]=0; m.clear_requested=true; }
        const auto& sections=pages[m.page].sections;
        m.section=(std::max)(0,(std::min)(m.section, int(sections.size())-1));
        if (!m.search[0] && !sections.empty()) {
            ImGui::Dummy(ImVec2(0,4));
            for (int i=0;i<int(sections.size());++i) {
                if (i && ImGui::GetContentRegionAvail().x > ImGui::CalcTextSize(sections[i]).x+32) ImGui::SameLine();
                ImGui::PushID(i);
                ImGui::PushStyleColor(ImGuiCol_Button,m.section==i ? ImVec4(.15f,.35f,.31f,1) : ImVec4(.09f,.12f,.15f,1));
                if (ImGui::Button(sections[i],ImVec2(0,30))) m.section=i;
                ImGui::PopStyleColor(); ImGui::PopID();
            }
        }
        ImGui::Dummy(ImVec2(0,8)); ImGui::Separator();
        ImGui::BeginChild("settings-content",ImVec2(0,-30),ImGuiChildFlags_AlwaysUseWindowPadding);
        content();
        ImGui::EndChild();
        ImGui::Separator();
        ImGui::TextDisabled(tr(m,"Changes apply immediately",u8"Изменения применяются сразу"));
        ImGui::EndChild();
    }
    ImGui::End();
}
}
