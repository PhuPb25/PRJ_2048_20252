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

    void saveHighestScore(uint32_t highestscore) ;// lưu lại điểm cao nhất
    uint32_t getHighScore() ;// lấy điểm cao nhất (Từ View gọi -> presenter -> Model -> View)

    // Lưu/ phục hồi trạng thái game đang chời
    // Khi vào lại có thế tiếp tục chơi thay vì tạo game cũ
    void saveGameState(uint32_t grid[4][4], uint32_t score);
    bool loadGameState(uint32_t grid[4][4], uint32_t& score); // trả về False nếu không có game nào được lưu

    // Gọi khi rời PlayScreen khi rời đi không muốn lưu game
    // Khi vào lại game sẽ là một game mới
    void clearGameState();

    void tick();
protected:
    ModelListener* modelListener;

    uint32_t highestScore = 0; // Điểm cao nhất

    //dư liệu game khi đang chơi, lưu lại khi rời PlayScreen, Khôi phục khi vào lại
    uint32_t savedGrid[4][4] = {0}; // lưu grid
    uint32_t savedScore = 0; // lưu điểm
    bool hasSavedGame = false; // lưu trạng thái
};

#endif // MODEL_HPP
