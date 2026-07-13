#include <gui/playscreen_screen/PlayScreenView.hpp>
#include <gui/playscreen_screen/PlayScreenPresenter.hpp>

PlayScreenPresenter::PlayScreenPresenter(PlayScreenView& v)
    : view(v)
{

}

void PlayScreenPresenter::activate()
{
	// activate chạy sau khi màn hình Play đã được active
	uint32_t loadedGrid[4][4];
	uint32_t loadedScore = 0;
	if(loadGameState(loadedGrid, loadedScore)) // kiểm tra xem có game đã lưu không
	{
		view.restoreGame(loadedGrid, loadedScore); // Load lại game cũ
	}
	else
	{
		view.khoitaogame(); // Khơi tạo game mới nếu không có game cũ
	}
}

void PlayScreenPresenter::deactivate()
{

}

int PlayScreenPresenter::getHighScore()
{
        return model->getHighScore();
}

void PlayScreenPresenter::saveHighScore(uint32_t score) // Lưu điểm cao nhất
{
	if(score > (model->getHighScore())) // gọi xuống lớp Model
	{
		model->saveHighestScore(score);
	}
}

// Lưu trạng thái game đang chời
void PlayScreenPresenter::saveGameState(uint32_t grid[4][4], uint32_t score)
{
	model->saveGameState(grid, score); // gọi xuống Model
}

// Khôi phục trang thái game đang chơi
bool PlayScreenPresenter::loadGameState(uint32_t grid[4][4], uint32_t& score)
{
	return model->loadGameState(grid, score); // Gọi xuống Model
}

// Xóa trang thái lưu
void PlayScreenPresenter::clearGameState()
{
	model->clearGameState(); // Gọi xuống Model
}
