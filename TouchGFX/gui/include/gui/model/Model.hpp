#ifndef MODEL_HPP
#define MODEL_HPP
#include <cstdint>
class ModelListener;

class Model
{
public:
    Model();

    void bind(ModelListener* listener)
    {
        modelListener = listener;
    }

    void saveHighestScore(uint32_t highestscore) ;// lưu điểm cao nhất
    uint32_t getHighScore() ;// lấy điểm cao nhất (Từ View gọi ->presenter-> Model -> View

    // luu/phuc hoi trang thai ban co dang choi, de khi back ve Home roi vao lai
    // PlayScreen thi tiep tuc duoc game cu thay vi luon bi tao moi
    void saveGameState(uint32_t grid[4][4], uint32_t score);
    bool loadGameState(uint32_t grid[4][4], uint32_t& score); // tra false neu chua co game nao duoc luu

    // goi khi roi PlayScreen MA van vua ket thuc (Game Over) - xoa co "co game da luu" de
    // lan sau vao lai PlayScreen se la 1 GAME MOI, khong phuc hoi lai ban co da chet (het nuoc di)
    void clearGameState();

    void tick();
protected:
    ModelListener* modelListener;

    uint32_t highestScore = 2; //lưu điểm cao nhất

    // du lieu game dang choi (luu khi roi PlayScreen, phuc hoi khi vao lai)
    uint32_t savedGrid[4][4] = {0};
    uint32_t savedScore = 0;
    bool hasSavedGame = false;
};

#endif // MODEL_HPP
