#include <gui/playscreen_screen/PlayScreenView.hpp>
#include <gui/playscreen_screen/PlayScreenPresenter.hpp>

PlayScreenPresenter::PlayScreenPresenter(PlayScreenView& v)
    : view(v)
{

}

void PlayScreenPresenter::activate()
{
	// activate() chay SAU khi model da duoc bind vao presenter, nen day moi la noi
	// an toan de goi model->loadGameState() (khac voi setupScreen(), luc do model co the chua bind).
	uint32_t loadedGrid[4][4];
	uint32_t loadedScore = 0;
	if(loadGameState(loadedGrid, loadedScore))
	{
		view.restoreGame(loadedGrid, loadedScore); // co game da luu => phuc hoi
	}
	else
	{
		view.khoitaogame(); // chua co game nao duoc luu => bat dau game moi
	}
}

void PlayScreenPresenter::deactivate()
{

}

int PlayScreenPresenter::getHighScore()
{
        return model->getHighScore();
}

void PlayScreenPresenter::saveHighScore(uint32_t score)// lưu điểm
{
	if(score > (model->getHighScore()))
	{
		model->saveHighestScore(score);
	}
}

// luu/phuc hoi trang thai ban co dang choi, cau noi View <-> Model
void PlayScreenPresenter::saveGameState(uint32_t grid[4][4], uint32_t score)
{
	model->saveGameState(grid, score);
}

bool PlayScreenPresenter::loadGameState(uint32_t grid[4][4], uint32_t& score)
{
	return model->loadGameState(grid, score);
}

void PlayScreenPresenter::clearGameState()
{
	model->clearGameState();
}
