#include <gui/playscreen_screen/PlayScreenView.hpp>
#include <cstring>
#include <cstdlib>
#include <cstdio>
//#include "stm32f4xx_hal.h"
#include <random>
#include "FreeRTOS.h"
#include "task.h"
#include <touchgfx/Color.hpp>
#include <algorithm>


using namespace std;

extern "C"
{
	extern volatile uint8_t newStatus;
	void Debug_UartLog(const char* msg, uint32_t len); // ham debug khai bao trong main.c
}

// bien chan doan: dem so lan setupScreen()/tearDownScreen() duoc goi, dung de
// xac nhan qua UART xem co dung 1 lan hay bi goi lai nhieu lan bat thuong
static uint32_t dbg_setupCount = 0;
static uint32_t dbg_teardownCount = 0;

//đăng ký callback
PlayScreenView::PlayScreenView():conTroCallBackX_Right(this, &PlayScreenView::callbackHandlerX_Right),
								 conTroCallBackY_Up(this, &PlayScreenView::callbackHandlerY_Up),
								 conTroCallBackX_Left(this, &PlayScreenView::callbackHandlerX_Left),
								 conTroCallBackY_Down(this, &PlayScreenView::callbackHandlerY_Down)
{

}

void PlayScreenView::setupScreen()
{
    PlayScreenViewBase::setupScreen();

    // LOG CHAN DOAN: in ra so lan setupScreen() duoc goi va trang thai cua co guard
    {
        dbg_setupCount++;
        char dbgBuf[64];
        int dbgLen = snprintf(dbgBuf, sizeof(dbgBuf), "[DBG] setupScreen #%lu guard=%d\r\n",
                               (unsigned long)dbg_setupCount, (int)daKhoiTaoContainer);
        Debug_UartLog(dbgBuf, dbgLen);
    }

    boxij[0][0] = &gridbox11;
    boxij[0][1] = &gridbox12;
    boxij[0][2] = &gridbox13;
    boxij[0][3] = &gridbox14;

    boxij[1][0] = &gridbox21;
    boxij[1][1] = &gridbox22;
    boxij[1][2] = &gridbox23;
    boxij[1][3] = &gridbox24;

    boxij[2][0] = &gridbox31;
    boxij[2][1] = &gridbox32;
    boxij[2][2] = &gridbox33;
    boxij[2][3] = &gridbox34;

    boxij[3][0] = &gridbox41;
    boxij[3][1] = &gridbox42;
    boxij[3][2] = &gridbox43;
    boxij[3][3] = &gridbox44;

    textAreaij[0][0] = &textArea11;
    textAreaij[0][1] = &textArea12;
    textAreaij[0][2] = &textArea13;
    textAreaij[0][3] = &textArea14;

    textAreaij[1][0] = &textArea21;
    textAreaij[1][1] = &textArea22;
    textAreaij[1][2] = &textArea23;
    textAreaij[1][3] = &textArea24;

    textAreaij[2][0] = &textArea31;
    textAreaij[2][1] = &textArea32;
    textAreaij[2][2] = &textArea33;
    textAreaij[2][3] = &textArea34;

    textAreaij[3][0] = &textArea41;
    textAreaij[3][1] = &textArea42;
    textAreaij[3][2] = &textArea43;
    textAreaij[3][3] = &textArea44;


    textBufferij[0][0] = textArea11Buffer;
    textBufferij[0][1] = textArea12Buffer;
    textBufferij[0][2] = textArea13Buffer;
    textBufferij[0][3] = textArea14Buffer;

    textBufferij[1][0] = textArea21Buffer;
    textBufferij[1][1] = textArea22Buffer;
    textBufferij[1][2] = textArea23Buffer;
    textBufferij[1][3] = textArea24Buffer;

    textBufferij[2][0] = textArea31Buffer;
    textBufferij[2][1] = textArea32Buffer;
    textBufferij[2][2] = textArea33Buffer;
    textBufferij[2][3] = textArea34Buffer;

    textBufferij[3][0] = textArea41Buffer;
    textBufferij[3][1] = textArea42Buffer;
    textBufferij[3][2] = textArea43Buffer;
    textBufferij[3][3] = textArea44Buffer;

    // containerij: Base chi sinh ra touchgfx::Container thuong (container11..container44),
    // khong ho tro animation. Vi MoveAnimator<T> la mixin phai duoc khai bao tu dau (ke thua T),
    // khong the "ep" 1 Container co san thanh MoveAnimator<Container>.
    // => Tu tao 16 instance MoveAnimator<Container> (containerij), chuyen gridbox/textArea
    //    tu container goc cua Base sang containerij, roi thay container goc bang containerij
    //    trong cay widget cua container1.
    //
    // QUAN TRONG: setupScreen() duoc TouchGFX goi lai MOI LAN man hinh nay duoc "switch in"
    // (vd: Back ve Home roi bam Play lai), KHONG CHI lan dau tien. Nhung PlayScreenView la
    // 1 instance duy nhat song suot doi chuong trinh (khong bi huy/tao lai giua cac lan chuyen
    // man hinh). Vi vay logic remove/add ben duoi CHI duoc phep chay 1 LAN DUY NHAT - neu chay
    // lai lan 2, cac widget (boxij/textAreaij) va containerij DA CO SAN trong cay se bi add lai
    // 1 lan nua, tao vong lap trong (linked list) cua Container. Khi do,
    // moi thao tac duyet cay widget (ve man hinh, xu ly tuong tac...) se lap vo han, lam
    // TouchGFX_Task chiem CPU vo thoi han va "treo" toan he thong FreeRTOS (ca task doc joystick/UART).
    if(!daKhoiTaoContainer)
    {
        Debug_UartLog("[DBG] BAT DAU di tru container (lan dau)\r\n", 42);

        touchgfx::Container* containerGocij[4][4] =
        {
            { &container11, &container12, &container13, &container14 },
            { &container21, &container22, &container23, &container24 },
            { &container31, &container32, &container33, &container34 },
            { &container41, &container42, &container43, &container44 }
        };

        for (int i = 0; i < 4; i++)
        {
            for (int j = 0; j < 4; j++)
            {
                // 1: lay gridbox/textArea ra khoi container goc (vi tri tuong doi giu nguyen 0,0)
                containerGocij[i][j]->remove(*boxij[i][j]);
                containerGocij[i][j]->remove(*textAreaij[i][j]);

                // 2: dat containerij dung kich thuoc/vi tri nhu container goc, roi add 2 widget con vao
                containerij[i][j].setXY(i * 55, j * 55);
                containerij[i][j].setWidthHeight(50, 50);
                containerij[i][j].add(*boxij[i][j]);
                containerij[i][j].add(*textAreaij[i][j]);

                // 3: thay container goc bang containerij trong container1 (container goc khong dung nua)
                container1.remove(*containerGocij[i][j]);
                container1.add(containerij[i][j]);
            }
        }

        daKhoiTaoContainer = true; // danh dau da khoi tao xong, khong cho chay lai logic tren nua

        Debug_UartLog("[DBG] XONG di tru container\r\n", 30);
    }
    else
    {
        Debug_UartLog("[DBG] BO QUA di tru container (guard chan)\r\n", 46);
    }

    Unicode::snprintf(showHighScoreBuffer, 10, "%u", presenter->getHighScore());
    showHighScore.invalidate();
}

void PlayScreenView::tearDownScreen()
{
    // LOG CHAN DOAN
    dbg_teardownCount++;
    {
        char dbgBuf[64];
        int dbgLen = snprintf(dbgBuf, sizeof(dbgBuf), "[DBG] tearDownScreen #%lu batXL=%d\r\n",
                              (unsigned long)dbg_teardownCount, (int)batXuLyLuoi);
        Debug_UartLog(dbgBuf, dbgLen);
    }

    // QUYET DINH co luu game hay khong khi roi man hinh:
    // 1) Neu nguoi dung vua bam "No" trong saveGameContainer (boQuaLuuKhiRoiManHinh == true)
    //    => KHONG luu, xoa luon ban co cu (clearGameState), bat ke batXuLyLuoi the nao.
    // 2) Nguoc lai (Yes, hoac roi man hinh bang cach khac nhu nut Back vat ly...):
    //    chi luu khi van CON DANG CHOI (batXuLyLuoi != 0). batXuLyLuoi == 0 nghia la ván vua
    //    ket thuc (Game Over) - khong luu lai ban co da chet, tranh lan sau "phuc hoi" mot
    //    ban co het nuoc di khien joystick khong gay thay doi gi (giong nhu bi treo).
    if(boQuaLuuKhiRoiManHinh)
    {
    	tinhDiem();
        presenter->clearGameState();

    }
    else if(batXuLyLuoi != 0)
    {
        presenter->saveGameState(grid, score);
    }
    else
    {
        presenter->clearGameState();
    }

    PlayScreenViewBase::tearDownScreen();
}

// code
void PlayScreenView::tinhDiem() // 0 khi người chơi bấm yesButton trong want end game
{
	if(dichuyenGa) return; // dang animation, bo qua de tranh xung dot trang thai

	batXuLyLuoi=2;
	// score da duoc cong don realtime trong qua trinh choi (xem congDiem()),
	// khong can quet lai grid de lay max tile nua
	updateHighestScore(); // b: lưu điểm cao nhất
}

void PlayScreenView::khoitaogame() // 1: khi bấm STARTbutton
{
	Debug_UartLog("[DBG] khoitaogame() - game moi\r\n", 33);
	dichuyenGa = false; // reset co animation khi bat dau 1 game moi
	batDauGame();
}

void PlayScreenView::xuLyYesButtonSave() // goi khi bam Yes trong saveGameContainer - roi man hinh VA luu
{
	// an popup di, khong de no con hien luc da chuyen sang HomeScreen
	saveGameContainer.setVisible(false);
	saveGameContainer.invalidate();

	// dam bao co nay la false (truong hop mac dinh) => tearDownScreen() se luu binh thuong
	// theo dung dieu kien cua no (chi luu khi batXuLyLuoi != 0)
	boQuaLuuKhiRoiManHinh = false;

	// chuyen ve HomeScreen.
	// LUU Y: ten ham "gotoHomeScreenScreenNoTransition()" la theo quy uoc dat ten chuan cua
	// TouchGFX Designer (Application -> goto<TenScreen>ScreenNoTransition/...TransitionXxx).
	// Mo file FrontendApplication.hpp (trong TouchGFX/generated/include/gui hoac tuong tu)
	// de xac nhan dung ten ham hien co trong project cua ban.
	application().gotoHomeScreenScreenNoTransition();
}

void PlayScreenView::xuLyNoButtonSave() // goi khi bam No trong saveGameContainer - roi man hinh nhung KHONG luu
{
	// an popup di
	saveGameContainer.setVisible(false);
	saveGameContainer.invalidate();

	// bao cho tearDownScreen() biet: LAN NAY KHONG duoc luu, du game van con dang choi
	boQuaLuuKhiRoiManHinh = true;

	// chuyen ve HomeScreen (cung 1 ham nhu nut Yes, chi khac co luu phia tren)
	application().gotoHomeScreenScreenNoTransition();

}

void PlayScreenView::restoreGame(uint32_t loadedGrid[4][4], uint32_t loadedScore) // phuc hoi game da luu, goi tu Presenter::activate()
{
	Debug_UartLog("[DBG] restoreGame() - phuc hoi game cu\r\n", 42);

	// LOG CHAN DOAN: in ra trang thai animation TRUOC khi reset, de xac nhan
	// gia thuyet "countToa bi ket > 0 do Back giua luc dang animation"
	{
		char dbgBuf[64];
		int dbgLen = snprintf(dbgBuf, sizeof(dbgBuf), "[DBG] truoc reset: countToa=%d batXL=%d dcGa=%d\r\n",
		                      countToa, (int)batXuLyLuoi, (int)dichuyenGa);
		Debug_UartLog(dbgBuf, dbgLen);
	}

	memcpy(grid, loadedGrid, sizeof(grid));
	score = loadedScore;

	Endcontainer.setVisible(false); // dam bao popup Game Over cua van truoc (neu con) duoc an di
	Endcontainer.invalidate();

	// QUAN TRONG: reset toan bo trang thai lien quan animation/joystick truoc khi cho phep choi tiep.
	// Neu nguoi dung bam Back dung luc dang co animation chay (countToa>0, dichuyenGa=true, Toa[][] con
	// du lieu cu), animation se bi "dong bang" giua duong vi PlayScreen khong active nua nen khong con
	// duoc goi handleTickEvent() => callback ket thuc animation KHONG BAO GIO duoc goi => countToa khong
	// bao gio tro ve 0 mot cach tu nhien. Neu khong reset o day, lan di chuyen joystick dau tien sau khi
	// quay lai se tinh countToa sai (cong don tren rac cu), khien finishAnimation() khong bao gio thay
	// countToa==0 => addRamdomBox()/batXuLyLuoi/dichuyenGa khong bao gio duoc mo lai => game bi khoa
	// vinh vien, joystick khong con tac dung (dung la trieu chung dang gap).
	countToa = 0;
	memset(Toa, -1, sizeof(Toa));
	yesAnimation = false;
	newSta = 0;
	oldSta = 0;

	// vtX0/vtY0 mac dinh la -1 tren doi tuong moi duoc tao (xem PlayScreenView.hpp), va
	// restoreGame() khong he luu/phuc hoi 2 gia tri nay (chi luu grid/score). Neu de -1,
	// lan di chuyen dau tien se truy cap boxij[-1][-1] (ngoai mang) => Undefined Behavior,
	// gay treo/loi khong xac dinh ngay tu lan keo joystick dau tien sau khi quay lai man hinh.
	// Da co kiem tra bien o moveRight/Left/Up/Down truoc khi dung vtX0/vtY0, nhung van nen
	// dat ve gia tri hop le ngay tu day de tranh phu thuoc hoan toan vao kiem tra rai rac do.
	vtX0 = 0;
	vtY0 = 0;

	// dua tat ca container ve dung vi tri pixel chuan theo i,j (phong truong hop mot container
	// dang o giua duong di chuyen luc bi Back, vi tri pixel se bi sai lech so voi grid logic)
	for (int i = 0; i < 4; i++)
	{
		for (int j = 0; j < 4; j++)
		{
			containerij[i][j].cancelMoveAnimation(); // huy animation dang treo (neu co) cua MoveAnimator
			containerij[i][j].clearMoveAnimationEndedAction(); // bo callback cu, tranh goi nham sau nay
			containerij[i][j].setXY(i * 55, j * 55); // tra ve dung vi tri chuan
		}
	}

	capNhatDiemHienTai();
	updateGiaoDien();
	batXuLyLuoi = 1; // cho phep tiep tuc choi
	dichuyenGa = false;
}

void PlayScreenView::batDauGame()
{
	Endcontainer.setVisible(false); // dam bao popup Game Over cua van truoc (neu con) duoc an di
	Endcontainer.invalidate();

	batXuLyLuoi=2;
	for(int i=0;i<4;i++)
	{
		for(int j=0;j<4;j++) grid[i][j]=0;
	}
	score = 0;
	capNhatDiemHienTai(); // reset hien thi diem realtime ve 0 khi bat dau game moi
	addRamdomBox();// b: lấy ngẫu nhiên 1 box để hiển thị như logic game
	updateGiaoDien(); // c: sau đó update giao diện thôi
	batXuLyLuoi=1; // kích hoạt sự kiện nghe Joystick
}

void PlayScreenView::congDiem(uint32_t diem) // goi moi khi 2 o duoc merge lai
{
	score += diem; // cong gia tri o vua duoc sinh ra (vd: 2+2=4 => cong 4 diem)
	capNhatDiemHienTai();
}

void PlayScreenView::capNhatDiemHienTai() // cap nhat hien thi diem hien tai (realtime) len man hinh
{
	// currentScoreTextBuffer: buffer chua chuoi (do Designer sinh ra), phai truyen buffer
	// vao snprintf, KHONG truyen widget currentScoreText truc tiep
	Unicode::snprintf(currentScoreTextBuffer, 10 , "%u", score);
	currentScoreText.invalidate();
}

void PlayScreenView::xuLyLuoi() // nghe trạng thái của joystick theo định kỳ để cập
{								 // nhật giao diện với 4 thao tác và xử lý endGame
	newSta = newStatus; // luôn lấy newStatus mỗi tick để oldSta không bị lệch trong lúc animation

	// VE CENTER -> reset toan bo "trang thai da xu ly": coi nhu lan day TIEP THEO (bat ke
	// huong nao) la 1 huong hoan toan moi.
	if(newSta == 0)
	{
		lapLaiCounter = 0;
		huongDaXuLy = 0;
	}

	if(batXuLyLuoi == 1 && newSta != 0) // chi xu ly khi KHONG dang animation va dang co huong duoc giu
	{
		// so voi huongDaXuLy (huong CUOI CUNG THUC SU duoc trigger), KHONG so voi oldSta -
		// vi oldSta van bi "doi" mu mo trong luc dang animation,
		// neu dung oldSta se bi tre 1 nhip moi khi doi huong giua luc con dang animation.
		bool huongMoi          = (newSta != huongDaXuLy);
		bool dangGiuVaHetKhoa   = (newSta == huongDaXuLy && lapLaiCounter == 0);

		if(huongMoi || dangGiuVaHetKhoa)
		{
			{
				char dbgBuf[80];
				int dbgLen = snprintf(dbgBuf, sizeof(dbgBuf), "[DBG] xuLyLuoi: newSta=%d countToa_truoc=%d lapLai=%d\r\n",
				                       newSta, countToa, (int)dangGiuVaHetKhoa);
				Debug_UartLog(dbgBuf, dbgLen);
			}

			memset(Toa,-1,sizeof(Toa));// gán Toa == -1

			batXuLyLuoi = 3; // giai đoạn Animation
			dichuyenGa = true; // đang trong 1 lượt di chuyển/animation => chặn input và các nút Yes/No/Close

			switch(newSta) // di chuyển
			{
				case 1: moveRight(); break;
				case 2: moveLeft(); break;
				case 3: moveUp(); break;
				case 4: moveDown(); break;
			}

			huongDaXuLy = newSta; // ghi nhan day la huong VUA duoc xu ly thuc su

			// DOI HUONG: luon mo khoa nhanh (INITIAL_HOLD_DELAY) cho LAN DAU cua huong nay -
			// dam bao doi huong giua luc dang giu luon duoc cam nhan la "ngay lap tuc", khong
			// bi delay nhu khi GIU TIEP cung 1 huong cu (REPEAT_DELAY chi ap dung khi
			// dangGiuVaHetKhoa, tuc la dang lap lai CUNG mot huong da xu ly truoc do).
			lapLaiCounter = huongMoi ? INITIAL_HOLD_DELAY : REPEAT_DELAY;
		}
	}

	else if(batXuLyLuoi == 2)// nếu kết thúc thì batXuLyLuoi == 0
	{
		batXuLyLuoi=0;
		endGame();
	}

	if(lapLaiCounter > 0) lapLaiCounter--; // giam dan moi tick, doc lap voi batXuLyLuoi

	oldSta = newSta; // van giu lai de tuong thich/debug, nhung KHONG con dung de quyet dinh trigger nua
}


void PlayScreenView::capNhatMauO(int x, int y, int giaTri)
{
    touchgfx::colortype color;

    switch(giaTri)
    {
        case 0:    color = touchgfx::Color::getColorFromRGB(255,255,255); break; // Ô trống
        case 2:    color = touchgfx::Color::getColorFromRGB(250,128,114); break;
        case 4:    color = touchgfx::Color::getColorFromRGB(233,150,122); break;
        case 8:    color = touchgfx::Color::getColorFromRGB(240,128,128); break;
        case 16:   color = touchgfx::Color::getColorFromRGB(205,92,92);  break;
        case 32:   color = touchgfx::Color::getColorFromRGB(220,20,60);  break;
        case 64:   color = touchgfx::Color::getColorFromRGB(178,34,34);  break;
        case 128:  color = touchgfx::Color::getColorFromRGB(64,224,208); break;
        case 256:  color = touchgfx::Color::getColorFromRGB(72,209,204); break;
        case 512:  color = touchgfx::Color::getColorFromRGB(0,206,209);  break;
        case 1024: color = touchgfx::Color::getColorFromRGB(32,178,170); break;
        case 2048: color = touchgfx::Color::getColorFromRGB(0,139,139);  break;
        default:   color = touchgfx::Color::getColorFromRGB(46,139,87);  break; // Từ 4096 trở lên
    }

    // Gán màu cho box và yêu cầu màn hình vẽ lại
    boxij[x][y]->setColor(color);
    boxij[x][y]->invalidate();
}

// 3: code thao tác
void PlayScreenView::moveRight()
{
	for(int y = 0; y < 4; y++)
	{
		for(int i=0;i<4;i++) gan[i][y]=0;

		int index = 3;//trỏ vào vị trí đấy của gán

		for(int x = 3; x >-1; x--)
		{
			if(grid[x][y] != 0) // có dịch chuyển
			{
				if(gan[index][y] == 0)// chưa có gì => gán được
				{
					gan[index][y] = grid[x][y];
					setDiChuyen(x,y,index,y);
				}
				else
				{
					if(gan[index][y] == grid[x][y]) // có gì rồi nhưng vẫn gán được
					{
						gan[index][y] = grid[x][y]*2;  // merge
						congDiem(gan[index][y]); // cong diem chuan 2048: cong gia tri o moi sinh ra
						setDiChuyen(x,y,index,y);
						index--;// giảm đi để không xét vị trí này nữa
					}
					else // có gì rồi nhưng không gán được thì bỏ
					{
						index--; // vị trí mới chắc chắn là gan[index]=0
						gan[index][y] = grid[x][y];
						setDiChuyen(x,y,index,y);
					}
				}
			}
		}
		// gán kết quả
		for(int i = 0; i<4; i++)	grid[i][y] = gan[i][y];
	}
	if(countToa != 0)
	{
		{
			char dbgBuf[64];
			int dbgLen = snprintf(dbgBuf, sizeof(dbgBuf), "[DBG] moveRight: countToa=%d -> bat dau animation\r\n", countToa);
			Debug_UartLog(dbgBuf, dbgLen);
		}
		// QUAN TRONG: vtX0/vtY0 co the dang la -1 (gia tri mac dinh ban dau, vd ngay sau khi
		// vua restoreGame() ma chua tung addRamdomBox() trong doi tuong nay) => phai kiem tra
		// bien truoc khi truy cap boxij[vtX0][vtY0], neu khong se truy cap ngoai mang (UB).
		if(vtX0 >= 0 && vtX0 < 4 && vtY0 >= 0 && vtY0 < 4)
		{
			capNhatMauO(vtX0, vtY0, grid[vtX0][vtY0]);
		}
		for(int y = 0; y < 4; y++) startX_RightMoveHinh(y);
	}
	else // 2 trường hợp, 1 là ko có animation, 2 là còn 1 ô cuối => cx ko có animation
	{
		Debug_UartLog("[DBG] moveRight: countToa=0 -> khong co animation\r\n", 53);
		batXuLyLuoi = 1;
		dichuyenGa = false; // ko co animation nao xay ra => khong co callback nao mo lai, phai tu mo khoa o day
	}
}
void PlayScreenView::moveLeft()
{
		for(int y = 0; y < 4; y++)
		{
			for(int i=0;i<4;i++) gan[i][y]=0;

			int index = 0;

			for(int x = 0; x <4; x++)
			{
				if(grid[x][y] != 0)
				{
					if(gan[index][y] == 0)
					{
						gan[index][y] = grid[x][y];
						setDiChuyen(x,y,index,y);
					}
					else
					{
						if(gan[index][y] == grid[x][y])
						{
							gan[index][y] = grid[x][y]*2;
							congDiem(gan[index][y]); // cong diem chuan 2048: cong gia tri o moi sinh ra
							setDiChuyen(x,y,index,y);
							index++;
						}
						else
						{
							index++;
							gan[index][y] = grid[x][y];
							setDiChuyen(x,y,index,y);
						}
					}
				}
			}
			for(int i = 0; i<4; i++) grid[i][y] = gan[i][y];

		}
		if(countToa != 0)
		{
		    if(vtX0 >= 0 && vtX0 < 4 && vtY0 >= 0 && vtY0 < 4) // xem giai thich o moveRight()
		    {
		    	capNhatMauO(vtX0, vtY0, grid[vtX0][vtY0]);
		    }
			for(int y = 0; y < 4; y++) startX_LeftMoveHinh(y);
		}
		else
		{
			batXuLyLuoi = 1;
			dichuyenGa = false; // ko co animation nao xay ra => khong co callback nao mo lai, phai tu mo khoa o day
		}
}
void PlayScreenView::moveUp()
{
		for(int x = 0; x < 4; x++)
		{
			for(int i=0;i<4;i++) gan[x][i]=0;

			int index = 0;

			for(int y = 0; y <4 ; y++)
			{
				if(grid[x][y] != 0)
				{
					if(gan[x][index] == 0)
					{
						gan[x][index] = grid[x][y];
						setDiChuyen(x,y,x,index);
					}
					else
					{
						if(gan[x][index] == grid[x][y])
						{
							gan[x][index] = grid[x][y]*2;
							congDiem(gan[x][index]); // cong diem chuan 2048: cong gia tri o moi sinh ra
							setDiChuyen(x,y,x,index);
							index++;
						}
						else
						{
							index++;
							gan[x][index] = grid[x][y];
							setDiChuyen(x,y,x,index);
						}
					}
				}
			}
			for(int i = 0; i<4 ; i++) grid[x][i] = gan[x][i];

		}
		if(countToa != 0)
		{
		    if(vtX0 >= 0 && vtX0 < 4 && vtY0 >= 0 && vtY0 < 4) // xem giai thich o moveRight()
		    {
		    	capNhatMauO(vtX0, vtY0, grid[vtX0][vtY0]);
		    }
			for(int x = 0; x < 4; x++) startY_UpMoveHinh(x);
		}
		else
		{
			batXuLyLuoi = 1;
			dichuyenGa = false; // ko co animation nao xay ra => khong co callback nao mo lai, phai tu mo khoa o day
		}
}
void PlayScreenView::moveDown()
{
			for(int x = 0; x < 4; x++)
			{
				for(int i=0;i<4;i++) gan[x][i]=0;

				int index = 3;

				for(int y = 3; y >-1 ; y--)
				{
					if(grid[x][y] != 0)
					{
						if(gan[x][index] == 0)
						{
							gan[x][index] = grid[x][y];
							setDiChuyen(x,y,x,index);
						}
						else
						{
							if(gan[x][index] == grid[x][y])
							{
								gan[x][index] = grid[x][y]*2;
								congDiem(gan[x][index]); // cong diem chuan 2048: cong gia tri o moi sinh ra
								setDiChuyen(x,y,x,index);
								index--;
							}
							else
							{
								index--;
								gan[x][index] = grid[x][y];
								setDiChuyen(x,y,x,index);
							}
						}
					}
				}
				for(int i = 0; i<4 ; i++)	grid[x][i] = gan[x][i];

			}
			if(countToa != 0)
			{
			    if(vtX0 >= 0 && vtX0 < 4 && vtY0 >= 0 && vtY0 < 4) // xem giai thich o moveRight()
			    {
			    	capNhatMauO(vtX0, vtY0, grid[vtX0][vtY0]);
			    }
				for(int x = 0; x < 4; x++) startY_DownMoveHinh(x);
			}
			else
			{
				batXuLyLuoi = 1;
				dichuyenGa = false; // ko co animation nao xay ra => khong co callback nao mo lai, phai tu mo khoa o day
			}
}

// 4: xử lý logic thêm box mới
void PlayScreenView::addRamdomBox()
{
	// Mảng lưu các vị trí có giá trị 0
	    int viTri0[16][2];
	    int zeroCount = 0;

	    // Tìm các ô có giá trị bằng 0
	    for (int x = 0; x < 4; x++)
	    {
	        for (int y = 0; y < 4; y++)
	        {
	            if (grid[x][y] == 0)
	            {
	                viTri0[zeroCount][0] = x;
	                viTri0[zeroCount][1] = y;
	                zeroCount++;
	            }
	        }
	    }

	    // nếu tạch
	    if(zeroCount == 0)
	    {
	    	 batXuLyLuoi = 2; // tín hiệu dừng
	    	 dichuyenGa = false; // het 1 luot, mo lai cho phep tuong tac (popup end game se hien ngay sau)
	    	 return;
	    }

	    // Lấy 1 vị trí ngẫu nhiên từ danh sách ô trống(sinh số ngẫu nhiên)
	    static mt19937 gen(xTaskGetTickCount()); // xTaskGetTickCount : thời gian chạy của hệ thống => hàm trả về 1 số ngẫu nhiên
	    uniform_int_distribution<> dis(0, zeroCount - 1); // tạo phân phối [0,zeroCount)

	    int randomIndex = dis(gen);
	    int randX = viTri0[randomIndex][0];
	    int randY = viTri0[randomIndex][1];

	    vtX0 = randX; // giữ giá trị này
	    vtY0 = randY;

	    // chon gia tri cho o moi: chuan 2048 goc dung ti le 90% ra "2", 10% ra "4".
	    // Dung phan phoi [1,10]: 1..9 (9/10 = 90%) -> 2, rieng 10 (1/10 = 10%) -> 4.
	    // Dung chung bo sinh so "gen" da co (chi tao distribution moi, khong tao lai generator
	    // de khong reset seed).
	    static uniform_int_distribution<> disGiaTri(1, 10);
	    uint32_t giaTriMoi = (disGiaTri(gen) <= 9) ? 2 : 4;

	    // Gán giá trị vừa chọn (2 hoặc 4) vào vị trí vừa chọn
	    grid[randX][randY] = giaTriMoi;

	    boxij[randX][randY]->setColor(touchgfx::Color::getColorFromRGB(255, 245, 157));// vàng
	    boxij[randX][randY]->invalidate();

		Unicode::snprintf(textBufferij[randX][randY], 10 , "%u", giaTriMoi);
		textAreaij[randX][randY]->invalidate();

	    // Nếu còn 1 ô cuối cùng thì xem xét còn đi được khôn
	    if (zeroCount == 1)
	    {
	    	if(kiemtra() == 0) // tức là không thể đi được nữa
	    	{
	    		batXuLyLuoi = 2;
	    		dichuyenGa = false; // het 1 luot, khong the di tiep => mo lai cho phep tuong tac
	    		return;
	    	}
	    }
	    batXuLyLuoi = 1;
	    dichuyenGa = false; // van con o trong, mo lai cho phep tuong tac luot tiep theo
}

// 4.1 kiểm tra còn đi tiếp đi được
uint8_t PlayScreenView::kiemtra()
{
	for(int i = 0; i < 4; i++) // x
	{
		for(int j = 0; j < 4; j++) // y
		{
			// tại vị trí i,j kiểm tra xem còn đi được nx ko
			if(i - 1 >= 0)
			{
				if(grid[i - 1][j] == grid[i][j]) return 1; // còn đi được
			}
			if(i + 1 < 4)
			{
				if(grid[i + 1][j] == grid[i][j]) return 1;
			}
			if(j - 1 >= 0)
			{
				if(grid[i][j - 1] == grid[i][j]) return 1;
			}
			if(j + 1 < 4)
			{
				if(grid[i][j + 1] == grid[i][j]) return 1;
			}
		}
	}
	return 0; // ko đi được
}

// 5: update giao diện
void PlayScreenView::updateGiaoDien()
{

		for(int i=0;i<4;i++)
		{
			for(int j=0;j<4;j++)
			{
				// 1. Cập nhật màu sắc dựa trên giá trị lưới
				capNhatMauO(i, j, grid[i][j]);

				// 2. Cập nhật text hiển thị
				if(grid[i][j] == 0)
				{
					Unicode::strncpy(textBufferij[i][j], "", TEXTAREA11_SIZE);
				}
				else
				{
					Unicode::snprintf(textBufferij[i][j], 10 , "%u", grid[i][j]);
				}
				textAreaij[i][j]->invalidate();

			}
		}
}

// 6: endGame
void PlayScreenView::endGame() // endGame khi thua
{
	batXuLyLuoi=0;
	Endcontainer.setVisible(true);// a: hiện container kết thúc
	// score da la tong diem cong don tu cac lan merge trong qua trinh choi (xem congDiem()),
	// khong can quet lai grid de lay max tile nua
	updateHighestScore(); // b: lưu điểm cao nhất
	Unicode::snprintf(YourScoreTextBuffer, 10 , "%u", score); // c: hiển thị điểm của người chơi

	YourScoreText.invalidate();
	Endcontainer.invalidate();
}

void PlayScreenView::updateHighestScore()
{
	presenter->saveHighScore(score);   // chi cap nhat RAM + danh dau dirty

	Unicode::snprintf(showHighScoreBuffer, 10, "%u", presenter->getHighScore());
	showHighScore.invalidate();
}


// 7: animation
//7.0 thêm vào hàng đợi chuỗi di chuyển
void PlayScreenView::setDiChuyen(int x1,int y1, int x2,int y2)
{
	if(x1 == x2 && y1 == y2) return;// ko có sự đi chuyển

	yesAnimation = true;
	countToa++; // tăng số Animation
	Toa[x1][y1] = (dichuyen){x1,y1,x2,y2}; // theo logic thì x1,y1 ở đầu là bé nhất
}

// 7.1
// 7.1.1 merge box
void PlayScreenView::mergeBox(int x2, int y2)
{
	// 1 đổi màu box x2,y2
	capNhatMauO(x2, y2, grid[x2][y2]);

	// 2 set giá trị
	Unicode::snprintf(textBufferij[x2][y2], 10 , "%u", grid[x2][y2]);
	textAreaij[x2][y2]->invalidate();
}
// 7.1.2 Animation về chỗ cũ
void PlayScreenView::setContainerVeChoCu(int i, int j)
{
	containerij[i][j].setXY(i*55, j*55);// set về chỗ chữ
	capNhatMauO(i, j, 0);
	Unicode::strncpy(textBufferij[i][j], "", TEXTAREA11_SIZE); // in ra rỗng

	textAreaij[i][j]->invalidate();
	boxij[i][j]->invalidate();
	containerij[i][j].invalidate();

	countToa--; // giảm số Animation hiện tại
}
// 7.1.3 finishAnimation
void PlayScreenView::finishAnimation()
{
	{
		char dbgBuf[48];
		int dbgLen = snprintf(dbgBuf, sizeof(dbgBuf), "[DBG] finishAnimation countToa=%d\r\n", countToa);
		Debug_UartLog(dbgBuf, dbgLen);
	}
	if(countToa == 0)// ko còn animation
	{
		addRamdomBox();
		updateGiaoDien(); // dam bao toan bo giao dien dong bo voi grid sau khi animation+merge xong
	}
}

// 7.2 kích hoạt quá trình di chuyển theo chiều ...

//7.2.1 Phải
void PlayScreenView::startX_RightMoveHinh(int y) // đi theo chiều ngang tại hàng thứ y
{

	for(int x = 3; x > -1 ; x--) // gặp box đầu tiên thì chạy
	{
		if(Toa[x][y].X1 != -1) // có Animation
		{
			int x2 = Toa[x][y].X2;
			int y2 = Toa[x][y].Y2;

			{
				char dbgBuf[64];
				int dbgLen = snprintf(dbgBuf, sizeof(dbgBuf), "[DBG] startX_Right: x=%d y=%d -> (%d,%d)\r\n", x, y, x2, y2);
				Debug_UartLog(dbgBuf, dbgLen);
			}

			// 1: move to front
			container1.remove(containerij[x][y]); // kỹ thuật để làm cho containerij có order cao
			container1.add(containerij[x][y]);

			// 2 xóa bóng
			containerij[x][y].clearMoveAnimationEndedAction(); // xóa vị trí ban đầu

			// 3 bật nghe animation và khởi tạo animation
			containerij[x][y].setMoveAnimationEndedAction(conTroCallBackX_Right);
			containerij[x][y].startMoveAnimation(x2*55, y2*55, timeAnimation, touchgfx::EasingEquations::linearEaseIn, touchgfx::EasingEquations::linearEaseIn);
			return;
		}
	}
	Debug_UartLog("[DBG] startX_Right: het, goi finishAnimation\r\n", 47);
	finishAnimation();
}
void PlayScreenView::callbackHandlerX_Right(const touchgfx::MoveAnimator<touchgfx::Container>& cont)
{
	Debug_UartLog("[DBG] callbackHandlerX_Right FIRED\r\n", 37);
	// tìm container đang gọi
	for(int i = 0; i < 4 ; i++)
	{
		for(int j = 0; j < 4; j++)
		{
			if(&containerij[i][j] == &cont)// tìm ra tọa độ i, j cái mà container đang gọi callback
			{
				mergeBox(Toa[i][j].X2, Toa[i][j].Y2); // merge ô
				setContainerVeChoCu(i, j);            // cho box đang animation về chỗ cũ

				Toa[i][j].X1 = -1;// đánh dấu là đi đc animation rồi
				startX_RightMoveHinh(j);
				return;
			}
		}
	}
	Debug_UartLog("[DBG] callbackHandlerX_Right: KHONG TIM THAY container!\r\n", 58);
}

//7.2.1 Trái
void PlayScreenView::startX_LeftMoveHinh(int y) // đi theo chiều ngang tại hang thứ i (y==i)
{
	for(int x = 0; x < 4 ; x++) // gặp box đầu tiên thì chạy
	{
		if(Toa[x][y].X1 != -1) // có Animation
		{
			int x2 = Toa[x][y].X2;
			int y2 = Toa[x][y].Y2;

			// 1: move to front
			container1.remove(containerij[x][y]); // kỹ thuật để làm cho containerij có order cao
			container1.add(containerij[x][y]);

			// 2 xóa bóng
			containerij[x][y].clearMoveAnimationEndedAction(); // xóa vị trí ban đầu

			// 3 bật nghe animation và khởi tạo animation
			containerij[x][y].setMoveAnimationEndedAction(conTroCallBackX_Left);
			containerij[x][y].startMoveAnimation(x2*55, y2*55, timeAnimation, touchgfx::EasingEquations::linearEaseIn, touchgfx::EasingEquations::linearEaseIn);
			return;
		}
	}
	finishAnimation();
}
void PlayScreenView::callbackHandlerX_Left(const touchgfx::MoveAnimator<touchgfx::Container>& cont)
{
	// tìm container đang gọi
	for(int i = 0; i < 4 ; i++)
	{
		for(int j = 0; j < 4; j++)
		{
			if(&containerij[i][j] == &cont)// tìm ra tọa độ i, j cái mà container đang gọi callback
			{
				mergeBox(Toa[i][j].X2, Toa[i][j].Y2); // merge ô
				setContainerVeChoCu(i, j);            // cho box đang animation về chỗ cũ

				Toa[i][j].X1 = -1;// đánh dấu là đi đc animation rồi
				startX_LeftMoveHinh(j);
				return;
			}
		}
	}
}


//7.2.1 Lên
void PlayScreenView::startY_UpMoveHinh(int x) // đi theo chiều dọc tại cột thứ i
{
	for(int y = 0; y < 4 ; y++) // gặp box đầu tiên thì chạy
	{
		if(Toa[x][y].X1 != -1) // có Animation
		{
			int x2 = Toa[x][y].X2;
			int y2 = Toa[x][y].Y2;

			// 1: move to front
			container1.remove(containerij[x][y]); // kỹ thuật để làm cho containerij có order cao
			container1.add(containerij[x][y]);

			// 2 xóa bóng
			containerij[x][y].clearMoveAnimationEndedAction(); // xóa vị trí ban đầu

			// 3 bật nghe animation và khởi tạo animation
			containerij[x][y].setMoveAnimationEndedAction(conTroCallBackY_Up);
			containerij[x][y].startMoveAnimation(x2*55, y2*55, timeAnimation, touchgfx::EasingEquations::linearEaseIn, touchgfx::EasingEquations::linearEaseIn);
			return;
		}
	}
	finishAnimation();
}
void PlayScreenView::callbackHandlerY_Up(const touchgfx::MoveAnimator<touchgfx::Container>& cont)
{
	for(int i = 0; i < 4 ; i++)
	{
		for(int j = 0; j < 4; j++)
		{
			if(&containerij[i][j] == &cont)// tìm ra tọa độ i, j cái mà container đang gọi callback
			{
				mergeBox(Toa[i][j].X2, Toa[i][j].Y2); // merge ô
				setContainerVeChoCu(i, j);            // cho box đang animation về chỗ cũ

				Toa[i][j].X1 = -1;// đánh dấu là đi đc animation rồi
				startY_UpMoveHinh(i);
				return;
			}
		}
	}
}


//7.2.1 Xuống
void PlayScreenView::startY_DownMoveHinh(int x) // đi theo chiều dọc tại cột thứ i
{
	for(int y = 3; y > -1 ; y--) // gặp box đầu tiên thì chạy
	{
		if(Toa[x][y].X1 != -1) // có Animation
		{
			int x2 = Toa[x][y].X2;
			int y2 = Toa[x][y].Y2;

			// 1: move to front
			container1.remove(containerij[x][y]); // kỹ thuật để làm cho containerij có order cao
			container1.add(containerij[x][y]);

			// 2 xóa bóng
			containerij[x][y].clearMoveAnimationEndedAction(); // xóa vị trí ban đầu

			// 3 bật nghe animation và khởi tạo animation
			containerij[x][y].setMoveAnimationEndedAction(conTroCallBackY_Down);
			containerij[x][y].startMoveAnimation(x2*55, y2*55, timeAnimation, touchgfx::EasingEquations::linearEaseIn, touchgfx::EasingEquations::linearEaseIn);
			return;
		}
	}
	finishAnimation();
}
void PlayScreenView::callbackHandlerY_Down(const touchgfx::MoveAnimator<touchgfx::Container>& cont)
{
	for(int i = 0; i < 4 ; i++)
	{
		for(int j = 0; j < 4; j++)
		{
			if(&containerij[i][j] == &cont)// tìm ra tọa độ i, j cái mà container đang gọi callback
			{
				mergeBox(Toa[i][j].X2, Toa[i][j].Y2); // merge ô
				setContainerVeChoCu(i, j);            // cho box đang animation về chỗ cũ

				Toa[i][j].X1 = -1;// đánh dấu là đi đc animation rồi
				startY_DownMoveHinh(i);
				return;
			}
		}
	}
}

void PlayScreenView::handleTickEvent()
{
    PlayScreenViewBase::handleTickEvent();

    // chi can goi xuLyLuoi() moi tick; xuLyLuoi() tu kiem tra batXuLyLuoi
    // de quyet dinh co xu ly joystick hay khong, khong can dieu kien rieng o day
    xuLyLuoi();
}
