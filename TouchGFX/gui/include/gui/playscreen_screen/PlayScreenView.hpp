#ifndef PLAYSCREENVIEW_HPP
#define PLAYSCREENVIEW_HPP

#include <gui_generated/playscreen_screen/PlayScreenViewBase.hpp>
#include <gui/playscreen_screen/PlayScreenPresenter.hpp>
#include <touchgfx/mixins/MoveAnimator.hpp>

class PlayScreenView : public PlayScreenViewBase
{
public:
    PlayScreenView();
    virtual ~PlayScreenView() {}
    virtual void setupScreen();
    virtual void tearDownScreen();

    virtual void xuLyLuoi(); // Xử lý input Joystick theo từng chu kì
    virtual void khoitaogame();// Khởi tạo 1 game mới
    virtual void restoreGame(uint32_t loadedGrid[4][4], uint32_t loadedScore); // Phục hồi lại game đã lưu
    virtual void tinhDiem();// Tính điểm khi kết thúc game
    virtual void xuLyYesButtonSave(); // Gọi khi bấm Yes khi xác nhận Lưu game
    virtual void xuLyNoButtonSave(); // Gọi khi bấm No xác nhận Ko Lưu Game
    virtual void handleTickEvent(); // Được Frame gọi mỗi tick, mỗi fame dùng để gọi xuLyLuoi() theo chu kỳ

protected:
    // Các hàm và biến nội bộ
    uint32_t grid[4][4];// Bàn cờ chính
    uint32_t gan[4][4];//  Bàn cờ phụ dùng khi di chuyên
    uint32_t score = 0; // Điểm hiện tại công dồn theo chuẩn 2048
    uint8_t newSta = 0; // Trạng thái JStick mới
    uint8_t oldSta = 0; // Trạng thái JStick cũ
    uint8_t batXuLyLuoi=0; // 0: game over, 1: đang chơi, 2: kết thúc lượt, 3: animation
    uint16_t timeAnimation = 6; // tốc độ animation di chuyển

    // Đây là guard (cờ bảo vệ) để đảm bảo đoạn code "di chuyển widget" (remove + add container) chỉ chạy đúng 1 lần duy nhất trong suốt thời gian chương trình chạy.
    bool daKhoiTaoContainer = false;

    // Đây là cờ để xử lý đặc biệt khi người dùng bấm nút "No" trong popup xác nhận lưu game khi thoát màn hình Play.
    bool boQuaLuuKhiRoiManHinh = false;

    void batDauGame(); // 1: bắt đầu game = khởi tạo game mới
    void addRamdomBox(); // 2: hiển thị 1 ô bất kì
    uint8_t kiemtra();

    void moveLeft(); // 3: dịch
    void moveRight(); // 3: dịch
    void moveUp();   // 3: dịch
    void moveDown(); // 3: dịch

    void updateGiaoDien(); // 4: update giao diện
    void endGame(); // 5 end game
    void updateHighestScore(); // 6: kết thúc thì lưu điểm cao nhất
    void capNhatMauO(int x, int y, int giaTri);

    // update version2.2 -- điểm số --
    void congDiem(uint32_t diem); // cộng điểm khi merge 2 ô (chuẩn 2048: cộng giá trị ô mới sinh ra)
    void capNhatDiemHienTai(); // cập nhật hiển thị điểm hiện tại (realtime) lên màn hình lúc đang chơi

    // Hiệu ứng di chuyển
    void setDiChuyen(int x1,int y1, int x2,int y2); // di chuyển từ tọa độ (x1,y1)->(x2,y2)
    void startX_RightMoveHinh(int i);
    void startX_LeftMoveHinh(int i);
    void startY_UpMoveHinh(int i);
    void startY_DownMoveHinh(int i);
    void setContainerVeChoCu(int i, int j);
    void mergeBox(int i,int j);
    void finishAnimation();

    // Tao mảng ánh xạ
    touchgfx::Box *boxij[4][4]; // mảng ô nền
    touchgfx::TextAreaWithOneWildcard *textAreaij[4][4]; // mảng vùng hiển thị chữ số
    touchgfx::MoveAnimator< touchgfx::Container > containerij[4][4]; // Mảng chứa 16 container có khả năng animation
    touchgfx::Unicode::UnicodeChar* textBufferij[4][4]; // chứa buffer chữ của từng textArea

    int vtX0 = -1, vtY0 = -1; // Vị trí ô random mới xuất hiện;

    // Tạo callback khi di chuyển

    // Lưu thông tin di chuyển
    struct dichuyen{
    	int X1, Y1, X2, Y2;
    };
    dichuyen Toa[4][4]; // mảng lưu các ô cần animation
    int countToa = 0;  // số animation hiện tại
    bool yesAnimation = false; // có animation không
    bool dichuyenGa = false; // True khi đang có 1 lượt di chuyển => Dùng d/Yes/No/Close, tranh xung dot trang thai


    touchgfx::Callback<PlayScreenView, const touchgfx::MoveAnimator<touchgfx::Container>&> conTroCallBackX_Right; 	// con trỏ di chuyển chiều ngang
    touchgfx::Callback<PlayScreenView, const touchgfx::MoveAnimator<touchgfx::Container>&> conTroCallBackY_Up; 		// con trỏ đi theo chiều dọc
    touchgfx::Callback<PlayScreenView, const touchgfx::MoveAnimator<touchgfx::Container>&> conTroCallBackX_Left; 	// con trỏ di chuyển chiều ngang
    touchgfx::Callback<PlayScreenView, const touchgfx::MoveAnimator<touchgfx::Container>&> conTroCallBackY_Down; 	// con trỏ đi theo chiều dọc

    void callbackHandlerX_Right(const touchgfx::MoveAnimator<touchgfx::Container>& cont);
    void callbackHandlerY_Up(const touchgfx::MoveAnimator<touchgfx::Container>& cont);
    void callbackHandlerX_Left(const touchgfx::MoveAnimator<touchgfx::Container>& cont);
    void callbackHandlerY_Down(const touchgfx::MoveAnimator<touchgfx::Container>& cont);


};

#endif // PLAYSCREENVIEW_HPP
