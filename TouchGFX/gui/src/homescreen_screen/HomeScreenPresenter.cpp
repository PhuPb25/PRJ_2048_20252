#include <gui/homescreen_screen/HomeScreenView.hpp>
#include <gui/homescreen_screen/HomeScreenPresenter.hpp>

HomeScreenPresenter::HomeScreenPresenter(HomeScreenView& v)
    : view(v)
{

}

void HomeScreenPresenter::activate()
{
	// activate được gọi sau khi Model được Bind vào Presenter
	view.getHighScore(); // gọi để cập nhật điểm cao nhât lên giao diện HomeScreen
}

void HomeScreenPresenter::deactivate()
{

}

uint32_t HomeScreenPresenter::getHighScore()
{
	return model->getHighScore(); // gọi xuống Model để lấy điểm cao nhất
}
