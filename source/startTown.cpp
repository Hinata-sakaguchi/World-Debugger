#include "Game.hpp"

void Game::startTown(void)
{
    renderPrepare();        
    m_camera->updateView();
    m_ground->render();
}
