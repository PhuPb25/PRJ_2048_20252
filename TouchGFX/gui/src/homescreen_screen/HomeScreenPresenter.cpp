#include <gui/homescreen_screen/HomeScreenView.hpp>
#include <gui/homescreen_screen/HomeScreenPresenter.hpp>

HomeScreenPresenter::HomeScreenPresenter(HomeScreenView& v)
    : view(v)
{

}

void HomeScreenPresenter::activate()
{
	// activate() chay SAU khi Model da duoc bind vao Presenter (xem giai thich tuong tu
	// trong PlayScreenPresenter::activate()), nen day moi la noi AN TOAN de goi ham can
	// truy cap model->getHighScore() - KHONG goi trong View::setupScreen() vi luc do model
	// co the chua bind, gay truy cap con tro null.
	//
	// Goi o day moi lan vao Home dam bao diem cao nhat luon duoc lam moi: vd vua choi xong
	// 1 van lap ky luc moi trong PlayScreen, quay ve Home se thay ngay so moi, khong bi
	// "dung" o gia tri cu.
	view.getHighScore();
}

void HomeScreenPresenter::deactivate()
{

}

uint32_t HomeScreenPresenter::getHighScore()
{
	return model->getHighScore();
}
