#include "../declarations.hpp"

#include <codecvt>

LRESULT CALLBACK main_default_wndproc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
    switch (message) 
    {
        case WM_COMMAND:
        {
            switch (int wmId = LOWORD(wParam); wmId)
            {
                case IDM_COPY_MAP:
                    copy_repr_to_clip(hWnd);
                    break;
                
                case IDM_PASTE_MAP:
                {
                    auto board_repr_sus = take_repr_from_clip(hWnd);
                    if (board_repr_sus.has_value()) {
                        turn = board_repr_sus.value().value.turn;
                        board.reset(std::move(board_repr_sus.value().value));
                        motion_input.clear();
                    }
                    else {
                        std::wstring error_message{ };

                        switch (board_repr_sus.error().type)
                        {
                            case ParseErrorType::General_EmptyString: error_message = L"General: Empty string"; break;
                            case ParseErrorType::Meta_CouldNotFindMeta:  error_message = L"Meta: Could not find meta"; break;
                            case ParseErrorType::Meta_InvalidVersion: error_message = L"Meta: Invalid version"; break;
                            case ParseErrorType::Meta_UnsupportedVersion: error_message = L"Meta: Unsupported version"; break;
                            case ParseErrorType::Meta_CouldNotFindHeight: error_message = L"Meta: Could not find height"; break;
                            case ParseErrorType::Meta_InvalidHeight: error_message = L"Meta: Invalid height"; break;
                            case ParseErrorType::Meta_CouldNotFindWidth: error_message = L"Meta: Could not find width"; break;
                            case ParseErrorType::Meta_InvalidWidth: error_message = L"Meta: Invalid width"; break;
                            case ParseErrorType::Meta_CouldNotFindIDW: error_message = L"Meta: Could not find idw"; break;
                            case ParseErrorType::Meta_CouldNotFindCurrentTurn: error_message = L"Meta: Could not find current turn"; break;
                            case ParseErrorType::Meta_InvalidCurrentTurn: error_message = L"Meta: Invalid current turn"; break;
                            case ParseErrorType::Meta_CouldNotFindCastlings: error_message = L"Meta: Could not find castlings"; break;
                            case ParseErrorType::Meta_InvalidCastling: error_message = L"Meta: Invalid castling"; break;
                            case ParseErrorType::Figure_UnexpectedEnd: error_message = L"Figure: Unexpected end"; break;
                            case ParseErrorType::Figure_IdDelimeterMissing: error_message = L"Figure: Id delimeter missing"; break;
                            case ParseErrorType::Figure_InvalidId: error_message = L"Figure: Invalid id"; break;
                            case ParseErrorType::Figure_InvalidPos: error_message = L"Figure: Invalid pos"; break;
                            case ParseErrorType::Figure_InvalidColor: error_message = L"Figure: Invalid color"; break;
                            case ParseErrorType::Figure_InvalidType: error_message = L"Figure: Invalid type"; break;
                            case ParseErrorType::MoveMessage_EmptyMap: error_message = L"MoveMessage: Empty map"; break;
                            case ParseErrorType::MoveMessage_CouldNotFindTo: error_message = L"MoveMessage: Could not find to"; break;
                            case ParseErrorType::MoveMessage_InvalidTo: error_message = L"MoveMessage: Invalid to"; break;
                            case ParseErrorType::MoveMessage_CouldNotFindPromotionChoice: error_message = L"MoveMessage: Could not find promotion choice"; break;
                            case ParseErrorType::MoveMessage_InvalidEnPassantToEatId: error_message = L"MoveMessage: Invalid en passant to eat id"; break;
                            case ParseErrorType::MoveMessage_InvalidPromotionChoice: error_message = L"MoveMessage: Invalid promitoin choice"; break;
                            case ParseErrorType::MoveMessage_CouldNotFindMainEvent: error_message = L"MoveMessage: Could not find main event"; break;
                            case ParseErrorType::SideEvent_EmptyString: error_message = L"SideEvent: Empty string"; break;
                            case ParseErrorType::SideEvent_CouldNotFindType: error_message = L"SideEvent: Could not find type"; break;
                            case ParseErrorType::SideEvent_InvalidType: error_message = L"SideEvent: Invalid type"; break;
                            case ParseErrorType::SideEvent_InvalidCastlingBreakId: error_message = L"SideEvent: Invalid castling break id"; break;
                            case ParseErrorType::MainEvent_CouldNotFindType: error_message = L"MainEvent: Could not find type"; break;
                            case ParseErrorType::MainEvent_InvalidType: error_message = L"MainEvent: Invalid type"; break;
                            case ParseErrorType::MainEvent_CouldNotFindCastlindSecondToMoveId: error_message = L"MainEvent: Could not find castling second to move id"; break;
                            case ParseErrorType::MainEvent_InvalidCastlingSecondToMoveId: error_message = L"MainEvent: Invalid castling second to move id"; break;
                            case ParseErrorType::MainEvent_CouldNotFindCastlingSecondInputFrom: error_message = L"MainEvent: Could not find castling second input from"; break;
                            case ParseErrorType::MainEvent_InvalidCastlingSecondInputFrom: error_message = L"MainEvent: Invalid castling second input from"; break;
                            case ParseErrorType::MainEvent_CouldNotFindCastlingSecondInputTo: error_message = L"MainEvent: Could not find castling second input to"; break;
                            case ParseErrorType::MainEvent_InvalidCastlingSecondInputTo: error_message = L"MainEvent: Invalind casling second input to"; break;
                            case ParseErrorType::MainEvent_CouldNotFindEnPassantEatenId: error_message = L"MainEvent: Could not find en passant eaten id"; break;
                            case ParseErrorType::MainEvent_InvalidEnPassantEatenId: error_message = L"MainEvent: Invalid en passant eaten id"; break;
                        }
                        MessageBox(hWnd, error_message.c_str(), L"Board repr parse error", MB_OK);

                        /* Debug print */ {
                            char error_message_utf8[1024];
                            WideCharToMultiByte(CP_UTF8, 0, error_message.c_str(), -1, error_message_utf8, 1024, NULL, NULL);
                            debug_print("Error:", error_message_utf8);
                            debug_print("\tBoard:", as_string<board_repr::BoardRepr>{}(board.get_repr(turn, true)));
                            debug_print("\tPos:", board_repr_sus.error().position);
                        }
                    }
                }
                    InvalidateRect(hWnd, NULL, NULL);
                    break;

                case IDM_WINDOW_MAKEQUAD:
                {
                    RECT rect;
                    GetWindowRect(hWnd, &rect);
                    
                    int quad_side = (rect.right - rect.left + rect.bottom - rect.top) / 2;
                    main_window.set_size(quad_side, quad_side + HEADER_HEIGHT);
                    SetWindowPos(hWnd, NULL, 
                        0, 0, quad_side, quad_side + HEADER_HEIGHT, SWP_NOZORDER | SWP_NOMOVE);
                }
                    break;
                    
                case IDM_EXIT:
                    // нужно ли чистить память?
                    DestroyWindow(hWnd);
                    break;
            } 
        }
            break;
           
        case WM_CREATE:
            update_main_window_title(hWnd);
            update_game_menu_variables(hWnd);
            break;

        case WM_LBUTTONDOWN:
            on_lbutton_down(hWnd, lParam);
            InvalidateRect(hWnd, NULL, NULL);
            break;

        case WM_MOVE:
            main_window.set_pos(lParam);
            break;
            
        case WM_SIZE:
            main_window.set_size(lParam);
            break;

        case WM_DESTROY:
            PostQuitMessage(0);
            break;
    }
    
    switch (window_state)
    {
        case WindowState::GAME:
            return mainproc::main_game_state_wndproc(hWnd, message, wParam, lParam);
        case WindowState::EDIT:
            return mainproc::main_edit_state_wndproc(hWnd, message, wParam, lParam);
        default:
            return static_cast<LRESULT>(0);
    }
}
