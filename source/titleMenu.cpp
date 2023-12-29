#include "Game.hpp"

void Game::titleMenu(u32 kDown, u32 kHeld, u32 kUp)
{
    renderPrepare();        
    m_camera->updateView();
    m_sea->render();
    m_ground->render();
    m_titleTown->setPosition(-10.4f, -3.5f, -5.0f);
    m_titleTown->render();
    m_titleTown->setPosition(-10.4f, -3.5f, 20.0f);
    m_titleTown->render();
    m_lamp->setScale(2.2);
    m_lamp->setPosition(-25.4, -2.5f, 16.0f);
    m_lamp->render();

    C2D_Prepare();  
    C2D_DrawSprite(&logoSprite);
    handleInput(kDown, kHeld, kUp);
    C2D_Flush();
}