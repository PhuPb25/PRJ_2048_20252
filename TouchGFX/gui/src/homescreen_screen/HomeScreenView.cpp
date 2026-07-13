#include <gui/homescreen_screen/HomeScreenView.hpp>

HomeScreenView::HomeScreenView()
{

}

void HomeScreenView::setupScreen()
{
    HomeScreenViewBase::setupScreen();
}

void HomeScreenView::tearDownScreen()
{
    HomeScreenViewBase::tearDownScreen();
}


void HomeScreenView::getHighScore()
{
	uint32_t x = presenter->getHighScore();
	Unicode::snprintf(ScoreTextBuffer, 16 , "%u", x);
	ScoreText.invalidate();
}
