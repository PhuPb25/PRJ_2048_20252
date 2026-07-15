#ifndef PLAYSCREENPRESENTER_HPP
#define PLAYSCREENPRESENTER_HPP

#include <gui/model/ModelListener.hpp>
#include <mvp/Presenter.hpp>

using namespace touchgfx;

class PlayScreenView;

class PlayScreenPresenter : public touchgfx::Presenter, public ModelListener
{
public:
    PlayScreenPresenter(PlayScreenView& v);

    /**
     * The activate function is called automatically when this screen is "switched in"
     * (ie. made active). Initialization logic can be placed here.
     */
    virtual void activate();

    /**
     * The deactivate function is called automatically when this screen is "switched out"
     * (ie. made inactive). Teardown functionality can be placed here.
     */
    virtual void deactivate();

    virtual ~PlayScreenPresenter() {}

    void saveHighScore(uint32_t score);// lưu điểm

    // Lưu/ Phục hồi trạng thái khi rời màn hình PlayScreen
    // Khi quay lại tiếp tục màn chơi cũ
    void saveGameState(uint32_t grid[4][4], uint32_t score);
    bool loadGameState(uint32_t grid[4][4], uint32_t& score);

    void commitHighScore();
    uint32_t getHighScore();

    // Gọi khi rời PlayScreen khi rời đi không muốn lưu game
    // Khi quay trở lại sẽ là màn hình game mới
    void clearGameState();
private:
    PlayScreenPresenter();

    PlayScreenView& view;
};

#endif // PLAYSCREENPRESENTER_HPP
