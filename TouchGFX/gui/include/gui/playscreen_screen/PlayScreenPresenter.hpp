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

    // luu/phuc hoi trang thai ban co dang choi, dung khi quay lai PlayScreen
    void saveGameState(uint32_t grid[4][4], uint32_t score);
    bool loadGameState(uint32_t grid[4][4], uint32_t& score);

    int getHighScore();
    // goi khi roi PlayScreen ma van da ket thuc (Game Over), de KHONG luu lai
    // ban co da chet - lan sau vao lai se la game moi
    void clearGameState();
private:
    PlayScreenPresenter();

    PlayScreenView& view;
};

#endif // PLAYSCREENPRESENTER_HPP
