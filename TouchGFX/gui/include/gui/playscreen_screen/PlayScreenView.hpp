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

    virtual void xuLyLuoi(); // gọi hàm theo chu kỳ từ giao TouchGFX Designer
    virtual void khoitaogame();// khởi tạo game
    virtual void restoreGame(uint32_t loadedGrid[4][4], uint32_t loadedScore); // phuc hoi game da luu (goi tu Presenter::activate())
    virtual void tinhDiem();// khi kết thúc game
    virtual void xuLyYesButtonSave(); // goi khi bam Yes trong saveGameContainer (xac nhan luu va roi PlayScreen)
    virtual void xuLyNoButtonSave(); // goi khi bam No trong saveGameContainer (roi PlayScreen nhung KHONG luu)
    virtual void handleTickEvent(); // được framework gọi mỗi tick (mỗi frame), dùng để gọi xuLyLuoi() theo chu kỳ

protected:
    // thêm các hàm và biến
    uint32_t grid[4][4];// dữ liệu các box
    uint32_t gan[4][4];// dữ liệu để tương tác với grid
    uint32_t score = 0; // điểm hiện tại (cộng dồn theo từng lần merge - chuẩn 2048)
    uint8_t newSta = 0; //
    uint8_t oldSta = 0;
    uint8_t batXuLyLuoi=0; // 0 thì là chưa làm gì, 1 là đang chơi, 2 là kết thúc
    uint16_t timeAnimation = 5;

    uint16_t lapLaiCounter = 0;                  // bo dem khoa lap lai, dung chung cho ca 2 giai doan duoi day
    static const uint16_t INITIAL_HOLD_DELAY = 30; // so tick PHAI GIU LIEN TUC (khong tha ve CENTER) truoc khi
                                                    // duoc tinh la "dang giu" va duoc phep lap lai lan dau tien.
                                                    // Day chinh la "do day" de tach 1 cu keo-tha nhanh (se KHONG
                                                    // bao gio dat duoc nguong nay) voi mot lan giu thuc su.
    static const uint16_t REPEAT_DELAY = 8;        // sau khi DA xac nhan la giu (qua duoc INITIAL_HOLD_DELAY),
                                                    // khoang cach giua cac lan lap lai tiep theo - ngan hon de
                                                    // cam giac muot khi giu lau.
    uint8_t huongDaXuLy = 0; // huong CUOI CUNG da thuc su duoc xu ly (trigger move), KHAC voi oldSta -
                              // oldSta van bi cap nhat moi tick ke ca luc dang animation (input bi chan),
                              // nen dung oldSta de so sanh se bi "nham" khi doi huong giua luc dang animation.
                              // huongDaXuLy chi doi gia tri dung 1 lan duy nhat ngay khi move duoc trigger.

    // flag DAM BAO logic "di tru" widget (remove khoi containerGocij, add vao containerij)
    // trong setupScreen() CHI CHAY DUNG 1 LAN trong suot doi chuong trinh.
    // Ly do: PlayScreenView la instance song suot doi chuong trinh (TouchGFX khong huy/tao lai
    // View moi lan chuyen man hinh), nhung setupScreen() lai duoc goi lai MOI LAN vao man hinh nay.
    // Neu khong co flag nay, lan vao lai thu 2 se add lai widget DA CO SAN trong containerij/container1,
    // gay vong lap trong danh sach lien ket don (linked list) cua Container => duyet cay vo han
    // => treo toan he thong (ca UART/joystick vi TouchGFX_Task chiem CPU vo thoi han).
    bool daKhoiTaoContainer = false;

    // flag CO bao cho tearDownScreen() biet KHONG duoc luu game lan nay, dung khi nguoi dung
    // bam "No" trong saveGameContainer (muon roi man hinh ma KHONG luu tien trinh).
    // Neu khong co flag nay, tearDownScreen() se tu dong luu binh thuong (vi logic mac dinh
    // cua no chi kiem tra batXuLyLuoi != 0), lam mat tac dung cua nut "No".
    bool boQuaLuuKhiRoiManHinh = false;

    void batDauGame(); // bắt đầu game = khởi tạo game mới
    void addRamdomBox(); // hiển thị 1 ô bất kì
    uint8_t kiemtra();

    void moveLeft(); // dịch
    void moveRight(); 
    void moveUp();   
    void moveDown(); 

    void updateGiaoDien(); 
    void endGame();
    void updateHighestScore(); // kết thúc thì lưu điểm cao nhất
    void capNhatMauO(int x, int y, int giaTri);

    // update -- điểm số --
    void congDiem(uint32_t diem); // cộng điểm khi merge 2 ô
    void capNhatDiemHienTai(); // cập nhật hiển thị điểm hiện tại (realtime) lên màn hình lúc đang chơi

    // hiệu ứng di chuyển 
    void setDiChuyen(int x1,int y1, int x2,int y2); // cài đặt di chuyển từ (x1,y1)->(x2,y2)
    void startX_RightMoveHinh(int i);
    void startX_LeftMoveHinh(int i);
    void startY_UpMoveHinh(int i);
    void startY_DownMoveHinh(int i);
    void setContainerVeChoCu(int i, int j);
    void mergeBox(int i,int j);
    void finishAnimation();

    // tạo ánh xạ i,j-> box_i_j;
    touchgfx::Box *boxij[4][4];
    touchgfx::TextAreaWithOneWildcard *textAreaij[4][4];
    touchgfx::MoveAnimator< touchgfx::Container > containerij[4][4]; // instance thuc su, tu quan ly trong View (Base chi co Container thuong, khong ho tro animation)
    touchgfx::Unicode::UnicodeChar* textBufferij[4][4];

    int vtX0 = -1, vtY0 = -1; // vị trí xuất hiện ô ramdom trước đó;

    // tạo callback cho animation

    struct dichuyen{
    	int X1, Y1, X2, Y2;
    };
    dichuyen Toa[4][4];
    int countToa = 0; // số animation hiện tại
    bool yesAnimation = false; // có animation không
    bool dichuyenGa = false; // true khi dang trong 1 luot di chuyen/animation => dung de chan input/Yes/No/Close


    touchgfx::Callback<PlayScreenView, const touchgfx::MoveAnimator<touchgfx::Container>&> conTroCallBackX_Right; // con trỏ di chuyển chiều ngang
    touchgfx::Callback<PlayScreenView, const touchgfx::MoveAnimator<touchgfx::Container>&> conTroCallBackY_Up; // con trỏ đi theo chiều dọc
    touchgfx::Callback<PlayScreenView, const touchgfx::MoveAnimator<touchgfx::Container>&> conTroCallBackX_Left; // con trỏ di chuyển chiều ngang
    touchgfx::Callback<PlayScreenView, const touchgfx::MoveAnimator<touchgfx::Container>&> conTroCallBackY_Down; // con trỏ đi theo chiều dọc

    void callbackHandlerX_Right(const touchgfx::MoveAnimator<touchgfx::Container>& cont);
    void callbackHandlerY_Up(const touchgfx::MoveAnimator<touchgfx::Container>& cont);
    void callbackHandlerX_Left(const touchgfx::MoveAnimator<touchgfx::Container>& cont);
    void callbackHandlerY_Down(const touchgfx::MoveAnimator<touchgfx::Container>& cont);


};

#endif // PLAYSCREENVIEW_HPP
