#ifndef IMGUI_DEFINE_MATH_OPERATORS
#define IMGUI_DEFINE_MATH_OPERATORS
#endif
#include <imgui.h>
#include <imgui_internal.h>
#include <functional>
#include <vector>
#include <string>
#include <compbuilder/CompBuilder.hpp>
#include <animations.hpp>
#include <unicodes.hpp>
#include "ChildManager.hpp"
#include "FontManager.hpp"
#include "LangManager.hpp"

using namespace ImGui;

void ChildManager::beginchild(const char* label, ImVec2 size) {
    current = label;
    const float width = size.x != 0 ? size.x : (GetWindowWidth() - GImGui->Style.WindowPadding.x*2 - GImGui->Style.ItemSpacing.x)*.5f;
    PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(16,16));
    PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(12,12));
    ImGuiChildFlags flags = ImGuiChildFlags_Borders | ImGuiChildFlags_AlwaysUseWindowPadding;
    if (size.y == 0) flags |= ImGuiChildFlags_AutoResizeY;
    BeginChild(label, ImVec2(width,size.y), flags);
    TextColored(GetStyleColorVec4(ImGuiCol_Scheme), "%s", LangManager::get().translate(label));
    Separator();
    PushItemWidth(-1);
}
void ChildManager::endchild() {
    PopItemWidth();
    EndChild();
    PopStyleVar(2);
}

void ChildManager::smoothscroll( ) {
	struct s {
		float scroll;
		float scroll_anim;
	}; auto& obj = anim_obj( GetCurrentWindow( )->Name, 23123, s{ } );

	obj.scroll = ImClamp( obj.scroll, 0.f, GetCurrentWindow( )->ScrollMax.y );
	obj.scroll_anim = ImClamp( obj.scroll_anim, 0.f, GetCurrentWindow( )->ScrollMax.y );

	ImGuiWindow* wheeling_window = nullptr;
	if ( GImGui->HoveredWindow ) {
		for ( ImGuiWindow* window = GImGui->HoveredWindow; window->Flags & ImGuiWindowFlags_ChildWindow; window = window->ParentWindow ) {
			if ( window->ScrollMax[ImGuiAxis_Y] == 0 )
				continue;

			wheeling_window = window;
		}
	}

	if ( !wheeling_window ) return;

	if ( wheeling_window == GetCurrentWindow( ) )
		obj.scroll = ImClamp( obj.scroll - GetIO( ).MouseWheel * 50, 0.f, GetCurrentWindow( )->ScrollMax.y );
	obj.scroll_anim = ImLerp( obj.scroll_anim, obj.scroll, GetIO( ).DeltaTime * 40 );
	GetCurrentWindow( )->Scroll.y = obj.scroll_anim;
}
