#include <gui/model/Model.hpp>
#include <gui/model/ModelListener.hpp>
#include <cstdint>
#include <cstring> // cho memcpy

// Các hàm đọc ghi Flash nội bộ được tạo ra trong main USER CODE 4
extern "C"
{
	uint32_t Flash_ReadHighScore(void);
	uint8_t Flash_SaveHighScore(uint32_t score);
}

Model::Model() : modelListener(0)
{
	highestScore = Flash_ReadHighScore(); // Điểm cao nhất hiện tại khởi tạo là 0 điểm
}

void Model::tick()
{

}

void Model::saveHighestScore(uint32_t score)
{
	if(score > highestScore)
	{
		highestScore = score;
		highScoreDirty = true;   // danh dau can ghi Flash, NHUNG chua ghi ngay
	}
}

void Model::commitHighScoreToFlash()
{
	if(!highScoreDirty) return;   // khong co gi moi -> tranh erase Flash vo ich (mon Flash)

	if(Flash_SaveHighScore(highestScore))
	{
		highScoreDirty = false;
	}
	// neu ghi that bai: giu highScoreDirty = true de thu lai lan sau
}

uint32_t Model::getHighScore()
{
	return highestScore; // Trả về điểm cao nhất
}

void Model::saveGameState(uint32_t grid[4][4], uint32_t score)
{
	memcpy(savedGrid, grid, sizeof(savedGrid)); // lưu lại grid hiện tại
	savedScore = score; // lưu lại điểm hiện tại
	hasSavedGame = true; // đặt cờ là đã lưu
}

bool Model::loadGameState(uint32_t grid[4][4], uint32_t& score)
{
	if(!hasSavedGame) return false; // nếu chưa save thì không có dữ liệu
	memcpy(grid, savedGrid, sizeof(savedGrid)); // khôi phục grid
	score = savedScore; // khôi phục điểm
	return true;
}

void Model::clearGameState()
{
	// xóa trang thái lưu game
	hasSavedGame = false;
}
