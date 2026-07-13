#include <gui/model/Model.hpp>
#include <gui/model/ModelListener.hpp>
#include <cstdint>
#include <cstring> // cho memcpy

// các hàm đọc/ghi Flash nội bộ, được cài đặt trong main.c (USER CODE BEGIN 4)
extern "C"
{
	uint32_t Flash_ReadHighScore(void);
	void Flash_SaveHighScore(uint32_t score);
}

Model::Model() : modelListener(0)
{
	highestScore = 0; // đọc điểm cao nhất đã lưu trong Flash khi khởi động (sống sót qua reset/mất điện)
}

void Model::tick()
{

}

void Model::saveHighestScore(uint32_t score)
{
	if(score > highestScore) // chỉ ghi Flash khi có điểm cao MỚI, tránh ghi liên tục gây mòn Flash
	{
		highestScore = score;
		Flash_SaveHighScore(highestScore);
	}
}
uint32_t Model::getHighScore()
{
	return highestScore;
}

void Model::saveGameState(uint32_t grid[4][4], uint32_t score)
{
	memcpy(savedGrid, grid, sizeof(savedGrid));
	savedScore = score;
	hasSavedGame = true;
}

bool Model::loadGameState(uint32_t grid[4][4], uint32_t& score)
{
	if(!hasSavedGame) return false;
	memcpy(grid, savedGrid, sizeof(savedGrid));
	score = savedScore;
	return true;
}

void Model::clearGameState()
{
	// chi can tat co nay, KHONG can xoa savedGrid/savedScore (du lieu cu se bi
	// ghi de boi lan saveGameState() hop le tiep theo, hoac khong bao gio duoc
	// doc lai vi loadGameState() da tra false ngay tu dau)
	hasSavedGame = false;
}
