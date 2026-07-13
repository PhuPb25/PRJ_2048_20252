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
	extern volatile uint8_t newStatus; // trạng thái của JStick
	void Debug_UartLog(const char* msg, uint32_t len); // hàm debug được khai báo trong
}

//	2 biến dùng để xem số lần màn hình bật tắt xem có gì bất thường không
static uint32_t dbg_setupCount = 0;
static uint32_t dbg_teardownCount = 0;

// Đăng ký hàm CallBack
// Các hàm này sẽ được gọi tự động khi hiệu ứng trượt (animation) của các khối số theo 4 hướng kết thúc
PlayScreenView::PlayScreenView():conTroCallBackX_Right(this, &PlayScreenView::callbackHandlerX_Right),
								 conTroCallBackY_Up(this, &PlayScreenView::callbackHandlerY_Up),
								 conTroCallBackX_Left(this, &PlayScreenView::callbackHandlerX_Left),
								 conTroCallBackY_Down(this, &PlayScreenView::callbackHandlerY_Down)
{

}

void PlayScreenView::setupScreen()
{
    PlayScreenViewBase::setupScreen();

    // Ghi LOG chẩn đoán số lần màn hình được gọi
    {
        dbg_setupCount++;
        char dbgBuf[64];
        int dbgLen = snprintf(dbgBuf, sizeof(dbgBuf), "[DBG] setupScreen #%lu guard=%d\r\n",
                               (unsigned long)dbg_setupCount, (int)daKhoiTaoContainer);
        Debug_UartLog(dbgBuf, dbgLen);
    }

    // đưa giao diện vào mảng 2 chiều để dễ dàng thao tác
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

    //Logic di trú container quan trọng: Vì thư viện chuẩn không hỗ trợ animation di chuyển,
    //lập trình viên đã tự tạo 16 MoveAnimator<Container>. Mã nguồn sẽ gỡ các ô số từ container cũ,
    //gắn sang container mới có hỗ trợ animation. Quá trình này được bảo vệ bởi cờ daKhoiTaoContainer
    //để đảm bảo chỉ chạy đúng 1 lần duy nhất, tránh gây vòng lặp vô hạn làm treo CPU
    if(!daKhoiTaoContainer)
    {
        Debug_UartLog("[DBG] BẮT ĐẦU DI TRÚ CONTAINER LẦN ĐẦU\r\n", 41);

        // Gom các container cũ vào một mảng
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
                // 1. lấy gridbox/textArea ra khỏi container gốc
                containerGocij[i][j]->remove(*boxij[i][j]);
                containerGocij[i][j]->remove(*textAreaij[i][j]);

                // 2: Setup cái vỏ mới: Định hình vỏ MoveAnimator mới (containerij) với kích thước 50x50.
                // Vị trí được nhân với 55 (tức là 50 pixel chiều rộng/cao + 5 pixel khoảng cách giữa các ô).
                // Đưa ruột vào vỏ mới: Lắp hình nền và chữ số vừa gỡ ở trên vào vỏ mới này.
                containerij[i][j].setXY(i * 55, j * 55);
                containerij[i][j].setWidthHeight(50, 50);
                containerij[i][j].add(*boxij[i][j]);
                containerij[i][j].add(*textAreaij[i][j]);

                // 3: code gỡ hoàn toàn cái vỏ cũ vứt đi (remove), và gắn cái vỏ mới vừa được nhồi ruột lên Bảng chơi (add)
                container1.remove(*containerGocij[i][j]);
                container1.add(containerij[i][j]);
            }
        }

        daKhoiTaoContainer = true; // danh dau da khoi tao xong, khong cho chay lai logic tren nua

        Debug_UartLog("[DBG] XONG DI TRÚ CONTAINER \r\n", 31);
    }
    else
    {
        Debug_UartLog("[DBG] BO QUA DI TRÚ CONTAINER \r\n", 33);
    }

    // Lấy điểm cao nhất từ Model và ghi ra màn hình
    Unicode::snprintf(showHighScoreBuffer, 10, "%u", presenter->getHighScore());
    showHighScore.invalidate();
}

void PlayScreenView::tearDownScreen()
{
    // LOG CHẨN ĐOÁN
    dbg_teardownCount++;
    {
        char dbgBuf[64];
        int dbgLen = snprintf(dbgBuf, sizeof(dbgBuf), "[DBG] tearDownScreen #%lu batXL=%d\r\n",
                              (unsigned long)dbg_teardownCount, (int)batXuLyLuoi);
        Debug_UartLog(dbgBuf, dbgLen);
    }

    // Quyết định xem có lưu ván này hay không
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

// CODE
void PlayScreenView::tinhDiem() // Khi người chơi bấm NO khi hỏi Save Game
{
	if(dichuyenGa) return; //đang animation bỏ quá để tránh xung đột

	batXuLyLuoi=2; // trạng thái kế thúc lượt
	// điểm đã được công dồn realtime rồi
	updateHighestScore(); // lưu điểm cao nhất
}

void PlayScreenView::khoitaogame() // Khi bấm STARTbutton
{
	Debug_UartLog("[DBG] kHỞI TAO GAME MỚI\r\n", 26);
	dichuyenGa = false; // Reset cờ animation khi bắt đầu game mới
	batDauGame();
}

void PlayScreenView::xuLyYesButtonSave() // Gọi khi bấm YES Save Game
{
	// Ản popup đi lúc nó chuyển về HomeScreen
	saveGameContainer.setVisible(false);
	saveGameContainer.invalidate();

	// Đảm bảo cờ này là false, để lưu màn hinh
	boQuaLuuKhiRoiManHinh = false;

	// Chuyển về HomeScreen
	// TouchGFX Designer (Application -> goto<TenScreen>ScreenNoTransition/...TransitionXxx).
	application().gotoHomeScreenScreenNoTransition();
}

void PlayScreenView::xuLyNoButtonSave() // Gọi khi bấm NO Save Game
{
	// Ản popup đi lúc nó chuyển về HomeScreen
	saveGameContainer.setVisible(false);
	saveGameContainer.invalidate();

	// Báo cho Teardown là lần này không lưu
	boQuaLuuKhiRoiManHinh = true;

	// Chuyển về HomeScreen
	// TouchGFX Designer (Application -> goto<TenScreen>ScreenNoTransition/...TransitionXxx).
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

// update version2.2 -- điểm số (chuẩn 2048: cộng dồn theo từng lần merge) --
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

void PlayScreenView::xuLyLuoi() // 2: nghe trạng thái của joystick theo định kỳ để cập
{								 // nhật giao diện với 4 thao tác và xử lý endGame
	// code và gọi các hàm moveUp, moveDown, moveLeft, moveRight
	// addRamdomBox()
	// updateGiaoDien()
	// updateHighestScore()
	newSta = newStatus; // luôn lấy newStatus mỗi tick để oldSta không bị lệch trong lúc animation

	if(batXuLyLuoi == 1) // nếu đã được kích hoạt thì bắt đầu xử lý
	{
		if(newSta != oldSta && newSta != 0) // nếu ng dùng thao tác joystick
		{
			{
				char dbgBuf[64];
				int dbgLen = snprintf(dbgBuf, sizeof(dbgBuf), "[DBG] xuLyLuoi: newSta=%d countToa_truoc=%d\r\n", newSta, countToa);
				Debug_UartLog(dbgBuf, dbgLen);
			}

			memset(Toa,-1,sizeof(Toa));// gán Toa == -1

			batXuLyLuoi = 3; // giai đoạn Animation
			dichuyenGa = true; // đang trong 1 lượt di chuyển/animation => chặn input và các nút Yes/No/Close

			switch(newSta) // di chuyển
			{
				case 1: moveRight(); break; // vừa di chuyển vừa update giao diện rồi
				case 2: moveLeft(); break;
				case 3: moveUp(); break;
				case 4: moveDown(); break;

				// các cv addRamdomBox, batXuLyLuoi để ở trong các hàm Animation
			}
		}
	}

	else if(batXuLyLuoi == 2)// nếu kết thúc thì batXuLyLuoi == 0
	{
		batXuLyLuoi=0;
		endGame();
	}

	oldSta = newSta; // luôn gán lại mỗi tick, kể cả khi đang animation (batXuLyLuoi==3), để không bỏ lỡ lượt nhả joystick
}

void PlayScreenView::capNhatMauO(int x, int y, int giaTri) // Tô màu cho từng ô theo giá trị tương ứng
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

	    // Gán giá trị 2 vào vị trí vừa chọn
	    grid[randX][randY] = 2;

	    capNhatMauO(randX, randY, 2);

		Unicode::snprintf(textBufferij[randX][randY], 10 , "%u", 2);
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

void PlayScreenView::updateHighestScore() // Gọi tới presenter
{
		presenter->saveHighScore(score);

		Unicode::snprintf(showHighScoreBuffer, 10, "%u", presenter->getHighScore());
		showHighScore.invalidate();
}


// Animation
// Thêm vào hàng đợi chuỗi di chuyển
void PlayScreenView::setDiChuyen(int x1,int y1, int x2,int y2)
{
	if(x1 == x2 && y1 == y2) return;// Nếu ko có sự đi chuyển thì thôi

	yesAnimation = true;
	countToa++; // tăng số Animation
	Toa[x1][y1] = (dichuyen){x1,y1,x2,y2}; // theo logic thì x1,y1 ở đầu là bé nhất
}

// Merge box
void PlayScreenView::mergeBox(int x2, int y2)
{
	// Đổi màu box x2,y2
	capNhatMauO(x2, y2, grid[x2][y2]);

	// Set giá trị
	Unicode::snprintf(textBufferij[x2][y2], 10 , "%u", grid[x2][y2]);
	textAreaij[x2][y2]->invalidate();
}
// Animation về chỗ cũ
void PlayScreenView::setContainerVeChoCu(int i, int j)
{
	containerij[i][j].setXY(i*55, j*55);// Set về chỗ cũ
	capNhatMauO(i, j, 0);
	Unicode::strncpy(textBufferij[i][j], "", TEXTAREA11_SIZE); // in ra rỗng

	textAreaij[i][j]->invalidate();
	boxij[i][j]->invalidate();
	containerij[i][j].invalidate();

	countToa--; // giảm số Animation hiện tại
}
// FinishAnimation
void PlayScreenView::finishAnimation()
{
	{
		char dbgBuf[48];
		int dbgLen = snprintf(dbgBuf, sizeof(dbgBuf), "[DBG] finishAnimation countToa=%d\r\n", countToa);
		Debug_UartLog(dbgBuf, dbgLen);
	}
	if(countToa == 0)// Ko còn animation
	{
		addRamdomBox();
		updateGiaoDien(); // Đảm báo giao điện được cập nhật
	}
}

// Kích hoạt quá trình di chuyển theo chiều

//Phải
void PlayScreenView::startX_RightMoveHinh(int y) // Đi theo chiều ngang tại hàng thứ y
{
	// Vòng lặp chạy ngược từ x = 3 về 0 (tức là quét từ PHẢI sang TRÁI trên hàng y)
	for(int x = 3; x > -1 ; x--)
	{
		// Kiểm tra xem ô tại tọa độ (x, y) này có lệnh di chuyển hay không
		// Mảng Toa[][] đã được tính toán trước đó, nếu X1 != -1 nghĩa là ô này cần trượt
		if(Toa[x][y].X1 != -1) //
		{
			int x2 = Toa[x][y].X2; 	// Lấy tọa độ X
			int y2 = Toa[x][y].Y2;	// Lấy tọa độ Y

			// LOG DEBUG
			{
				char dbgBuf[64];
				int dbgLen = snprintf(dbgBuf, sizeof(dbgBuf), "[DBG] startX_Right: x=%d y=%d -> (%d,%d)\r\n", x, y, x2, y2);
				Debug_UartLog(dbgBuf, dbgLen);
			}

			// Trong TouchGFX, widget nào được add vào sau cùng sẽ nằm ở lớp trên cùng.
			// Bằng cách xóa đi (remove) rồi thêm lại (add), ô số đang trượt này sẽ đè lên
			// các ô số khác khi nó đi ngang qua, không bị hiện tượng "chui" dưới ô khác.
			container1.remove(containerij[x][y]);
			container1.add(containerij[x][y]);

			// Xóa bỏ các cài đặt sự kiện kết thúc chuyển động cũ để tránh xung đột
			containerij[x][y].clearMoveAnimationEndedAction();

			// ĐĂNG KÝ CALLBACK & KÍCH HOẠT TRƯỢT
			// Cài đặt: Khi nào ô này trượt tới đích, hãy tự động gọi hàm `callbackHandlerX_Right`
			containerij[x][y].setMoveAnimationEndedAction(conTroCallBackX_Right);

			// Bắt đầu trượt: Tọa độ đích pixel = Tọa độ lưới * 55.
			// `timeAnimation` là thời gian trượt, sử dụng gia tốc tuyến tính (linearEaseIn)
			containerij[x][y].startMoveAnimation(x2*55, y2*55, timeAnimation, touchgfx::EasingEquations::linearEaseIn, touchgfx::EasingEquations::linearEaseIn);
			return;
		}
	}
	// Nếu vòng lặp chạy hết từ 3 về 0 mà không tìm thấy ô nào có lệnh trượt nữa
	// (tức là tất cả các ô trong hàng này đã trượt xong hoặc hàng này không có ô nào cần đi)
	Debug_UartLog("[DBG] startX_Right: het, goi finishAnimation\r\n", 47);
	finishAnimation(); // Gọi hàm kết thúc để sinh số mới hoặc mở khóa cho Joystick
}

void PlayScreenView::callbackHandlerX_Right(const touchgfx::MoveAnimator<touchgfx::Container>& cont)
{
    Debug_UartLog("[DBG] callbackHandlerX_Right FIRED\r\n", 37);

    // VÌ TouchGFX chỉ báo là "có một cái hộp vừa trượt xong" thông qua biến `cont`
    // Nên ta phải dùng 2 vòng lặp để dò xem cái hộp `cont` đó là ô nào (i, j) trên bàn cờ 4x4
    for(int i = 0; i < 4 ; i++)
    {
        for(int j = 0; j < 4; j++)
        {
            if(&containerij[i][j] == &cont) // Đã tìm thấy tọa độ gốc (i, j) của chiếc hộp vừa trượt!
            {
                // MẸO 2: "ẢO THUẬT" HOÁN ĐỔI GIAO DIỆN
                // 1. Cập nhật giá trị mới cho ô ĐÍCH (Toa[i][j].X2, Toa[i][j].Y2).
                // Ví dụ nếu hai ô '2' chập vào nhau, hàm mergeBox sẽ biến ô đích thành '4' và đổi màu.
                mergeBox(Toa[i][j].X2, Toa[i][j].Y2);

                // 2. GIẬT Ô CŨ VỀ VỊ TRÍ BAN ĐẦU:
                // Chiếc hộp containerij[i][j] thực chất vừa bị trượt sang vị trí mới trên màn hình.
                // Hàm setContainerVeChoCu sẽ ngay lập tức "bốc" chiếc hộp này đặt phịch về vị trí (i, j) gốc,
                // đồng thời xóa chữ số bên trong nó đi (biến nó thành ô trống).
                setContainerVeChoCu(i, j);

                // Đánh dấu ô (i, j) này đã hoàn thành xong hiệu ứng animation
                Toa[i][j].X1 = -1;

                // HIỆU ỨNG DOMINO: Gọi lại hàm di chuyển cho HÀNG HIỆN TẠI (hàng j)
                // Để nó tiếp tục tìm xem còn ô nào phía sau (bên trái) cần trượt nữa không.
                startX_RightMoveHinh(j);

                return; // Hoàn thành nhiệm vụ cho ô này, thoát hàm.
            }
        }
    }
    // Đoạn phòng hờ lỗi hệ thống
    Debug_UartLog("[DBG] callbackHandlerX_Right: KHONG TIM THAY container!\r\n", 58);
}

//Trái
void PlayScreenView::startX_LeftMoveHinh(int y) // đi theo chiều ngang tại hang thứ i (y==i)
{
    // Vòng lặp chạy từ x = 0 tăng dần đến x = 3 (Quét từ TRÁI sang PHẢI trên hàng y)
    // Khi gạt sang TRÁI, ô nào ở gần bên trái hơn sẽ được ưu tiên trượt trước.
    for(int x = 0; x < 4 ; x++)
    {
        // Kiểm tra mảng Toạ độ xem ô tại vị trí (x, y) này có lệnh di chuyển hay không.
        // Nếu Toa[x][y].X1 khác -1, nghĩa là ô này có Animation cần chạy.
        if(Toa[x][y].X1 != -1)
        {
            // Lấy ra toạ độ đích (x2, y2) mà ô này cần phải trượt tới
            int x2 = Toa[x][y].X2;
            int y2 = Toa[x][y].Y2;

            // ---- Bước 1: Kỹ thuật đưa Ô lên lớp trên cùng (Z-order) ----
            // Trong TouchGFX, Widget nào được add vào sau cùng sẽ hiển thị đè lên trên.
            // Đoạn code này gỡ ô số ra khỏi container cha (container1) rồi gắn lại ngay lập tức,
            // giúp ô này nằm ở lớp trên cùng, khi trượt qua các ô khác sẽ không bị chui xuống dưới đáy.
            container1.remove(containerij[x][y]);
            container1.add(containerij[x][y]);

            // ---- Bước 2: Xóa bóng (Hành động cũ) ----
            // Xóa bỏ các cài đặt sự kiện kết thúc animation cũ của ô này (nếu có) để tránh xung đột lỗi.
            containerij[x][y].clearMoveAnimationEndedAction();

            // ---- Bước 3: Đăng ký hàm xử lý khi trượt xong & Khởi chạy ----
            // Cài đặt: Khi chiếc hộp này trượt đến đích thành công, hãy tự động kích hoạt hàm 'conTroCallBackX_Left'
            containerij[x][y].setMoveAnimationEndedAction(conTroCallBackX_Left);

            // Ra lệnh cho phần cứng bắt đầu trượt: Tọa độ pixel đích = Toạ độ lưới * 55 pixel.
            // 'timeAnimation' là thời gian trượt. Sử dụng công thức toán học 'linearEaseIn' để chuyển động đều mượt mà.
            containerij[x][y].startMoveAnimation(x2*55, y2*55, timeAnimation, touchgfx::EasingEquations::linearEaseIn, touchgfx::EasingEquations::linearEaseIn);

            // CỰC KỲ QUAN TRỌNG: Kích hoạt xong ô đầu tiên này là THOÁT HÀM LUÔN (return),
            // không cho các ô phía sau trượt cùng lúc nhằm tạo hiệu ứng xếp hàng (Domino).
            return;
        }
    }

    // Nếu vòng lặp chạy từ 0 đến 3 mà không lọt vào lệnh 'if' bên trên
    // (tức là hàng này không có ô nào cần trượt, hoặc tất cả các ô trong hàng đã trượt xong xuôi)
    finishAnimation(); // Gọi hàm kết thúc hiệu ứng chung để sinh số mới hoặc mở khóa Joystick.
}

void PlayScreenView::callbackHandlerX_Left(const touchgfx::MoveAnimator<touchgfx::Container>& cont)
{
    // Hệ thống chỉ trả về một biến chung chung là 'cont' (chiếc hộp vừa trượt xong).
    // Do đó, ta phải dùng 2 vòng lặp để dò tìm trong ma trận 4x4 xem 'cont' này là ô nào.
    for(int i = 0; i < 4 ; i++)
    {
        for(int j = 0; j < 4; j++)
        {
            // So sánh địa chỉ bộ nhớ, nếu trùng khớp -> Tìm ra toạ độ gốc (i, j) của ô vừa trượt xong!
            if(&containerij[i][j] == &cont)
            {
                // 1. Cập nhật giá trị hiển thị cho ô ĐÍCH (Ví dụ: ô đích nhận giá trị mới hoặc gộp thành số lớn hơn)
                mergeBox(Toa[i][j].X2, Toa[i][j].Y2);

                // 2. Vì chiếc hộp containerij[i][j] thực tế vừa bị dịch chuyển sang vị trí mới trên màn hình.
                // Hàm này sẽ "bốc" chiếc hộp đó đặt giật lùi về lại đúng vị trí pixel gốc (i, j) của nó,
                // đồng thời xóa chữ số bên trong (biến nó thành ô trống sẵn sàng cho lượt chơi sau).
                setContainerVeChoCu(i, j);

                // 3. Đánh dấu ô (i, j) này đã hoàn tất Animation (gán bằng -1 để vòng lặp quét sau bỏ qua nó)
                Toa[i][j].X1 = -1;

                // 4. HIỆU ỨNG CHUỖI (Domino): Tiếp tục gọi lại hàm di chuyển cho hàng hiện tại (hàng j)
                // Để nó quét tiếp từ trái sang phải xem còn ô nào đang xếp hàng chờ trượt nữa không.
                startX_LeftMoveHinh(j);

                // Xử lý xong ô này, thoát hàm.
                return;
            }
        }
    }
}


//7.2.1 Lên
void PlayScreenView::startY_UpMoveHinh(int x)
{
    // Quét từ y = 0 đến y = 3 (Từ TRÊN xuống DƯỚI trong cùng cột x).
    // Ô nào ở trên cao hơn sẽ được quét trúng trước và ưu tiên trượt trước.
    for(int y = 0; y < 4 ; y++)
    {
        // Kiểm tra xem ô (x, y) này có được lệnh di chuyển không
        if(Toa[x][y].X1 != -1)
        {
            // Lấy tọa độ đích cần trượt tới
            int x2 = Toa[x][y].X2; // Thực ra x2 luôn bằng x vì trượt dọc
            int y2 = Toa[x][y].Y2;

            // 1: Đưa ô này lên lớp hiển thị trên cùng (tránh bị ô khác đè lên khi trượt ngang qua)
            container1.remove(containerij[x][y]);
            container1.add(containerij[x][y]);

            // 2: Xóa các thiết lập kết thúc animation cũ để dọn đường
            containerij[x][y].clearMoveAnimationEndedAction();

            // 3: Cài đặt: Khi trượt xong, hãy gọi hàm 'conTroCallBackY_Up'
            containerij[x][y].setMoveAnimationEndedAction(conTroCallBackY_Up);

            // 4: Bắt đầu hiệu ứng trượt tới tọa độ pixel (x2*55, y2*55)
            containerij[x][y].startMoveAnimation(x2*55, y2*55, timeAnimation, touchgfx::EasingEquations::linearEaseIn, touchgfx::EasingEquations::linearEaseIn);

            // Đã kích hoạt xong ô đầu tiên tìm thấy -> Thoát hàm ngay lập tức để ô này chạy.
            // Các ô phía dưới nó (y lớn hơn) phải chờ.
            return;
        }
    }
    // Nếu quét hết cột mà không có ô nào cần trượt nữa -> kết thúc animation chung.
    finishAnimation();
}

void PlayScreenView::callbackHandlerY_Up(const touchgfx::MoveAnimator<touchgfx::Container>& cont)
{
    // Dùng 2 vòng lặp để dò tìm xem 'cont' (chiếc hộp vừa trượt xong) là ô (i, j) nào trên bàn cờ.
    for(int i = 0; i < 4 ; i++)
    {
        for(int j = 0; j < 4; j++)
        {
            if(&containerij[i][j] == &cont) // Tìm thấy!
            {
                // Cập nhật giá trị và màu sắc cho ô ĐÍCH (nơi chiếc hộp vừa bay tới)
                mergeBox(Toa[i][j].X2, Toa[i][j].Y2);

                // Trả cái vỏ rỗng của chiếc hộp vừa trượt về lại đúng tọa độ xuất phát ban đầu
                setContainerVeChoCu(i, j);

                // Đánh dấu ô (i, j) này đã xử lý xong hiệu ứng
                Toa[i][j].X1 = -1;

                // Gọi lại chính hàm kích hoạt cột i (chú ý ở đây i đóng vai trò là x).
                // Nó sẽ quét lại cột này từ trên xuống dưới để lôi ô tiếp theo ra trượt (Hiệu ứng Domino).
                startY_UpMoveHinh(i);

                // Xong việc, thoát hàm dò tìm.
                return;
            }
        }
    }
}


//7.2.1 Xuống
void PlayScreenView::startY_DownMoveHinh(int x)
{
    // KHÁC BIỆT CHÍNH LÀ Ở ĐÂY: Vòng lặp chạy ngược từ y = 3 về y = 0
    // Tức là quét từ DƯỚI đáy lên TRÊN đỉnh.
    // Ô nào ở sát đáy bàn cờ hơn sẽ được ưu tiên đẩy đi trước để dọn chỗ cho các ô bên trên rơi xuống.
    for(int y = 3; y > -1 ; y--)
    {
        // Các bước còn lại y hệt như trượt Lên: Kiểm tra xem có lệnh trượt không.
        if(Toa[x][y].X1 != -1)
        {
            int x2 = Toa[x][y].X2;
            int y2 = Toa[x][y].Y2;

            // Đưa lên lớp hiển thị trên cùng
            container1.remove(containerij[x][y]);
            container1.add(containerij[x][y]);

            // Xóa rác cài đặt cũ
            containerij[x][y].clearMoveAnimationEndedAction();

            // Cài đặt callback đích danh cho hướng XUỐNG (conTroCallBackY_Down)
            containerij[x][y].setMoveAnimationEndedAction(conTroCallBackY_Down);

            // Ra lệnh bắt đầu trượt
            containerij[x][y].startMoveAnimation(x2*55, y2*55, timeAnimation, touchgfx::EasingEquations::linearEaseIn, touchgfx::EasingEquations::linearEaseIn);

            // Kích hoạt xong 1 ô -> Thoát hàm.
            return;
        }
    }
    // Hết cột -> Kết thúc.
    finishAnimation();
}

void PlayScreenView::callbackHandlerY_Down(const touchgfx::MoveAnimator<touchgfx::Container>& cont)
{
    // Dò tìm ô (i, j) vừa trượt xong
    for(int i = 0; i < 4 ; i++)
    {
        for(int j = 0; j < 4; j++)
        {
            if(&containerij[i][j] == &cont) // Tìm thấy!
            {
                // Cập nhật ô đích (có thể là gộp số)
                mergeBox(Toa[i][j].X2, Toa[i][j].Y2);

                // Lôi cổ cái vỏ rỗng về vị trí xuất phát
                setContainerVeChoCu(i, j);

                // Đánh dấu đã xong
                Toa[i][j].X1 = -1;

                // Gọi lại hàm quét cột i từ Dưới lên Trên để xem còn ô nào mắc kẹt ở trên cần rơi xuống tiếp không.
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
