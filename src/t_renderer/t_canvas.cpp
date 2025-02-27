#include "t_renderer/t_canvas.hpp"

#include "ftxui/dom/canvas.hpp"
#include "t_renderer/t_current_theme.hpp"

namespace Tetris::Renderer
{
void Canvas::drawTetromino(ftxui::Canvas &canvas, Tetris::Game::Tetromino tetromino, bool isCentered, bool isShadow)
{
    auto data = tetromino.getData();

    for (int row = 0; row < (int)data.size(); row++)
    {
        for (int col = 0; col < (int)data[row].size(); col++)
        {
            if (data[row][col] == Tetris::Game::BlockType::BLOCK)
            {
                Canvas::drawBlock(
                    canvas,
                    (!isCentered ? (int)tetromino.currentPosition.x : 1) + col,
                    (!isCentered ? (int)tetromino.currentPosition.y : 1) + row,
                    tetromino.color,
                    isCentered,
                    isShadow,
                    tetromino.type
                );
            }
        }
    }
}

void Canvas::drawBoard(
    ftxui::Canvas                                         &canvas,
    std::vector<std::vector<Tetris::Game::BoardBlockType>> board,
    std::vector<std::vector<ftxui::Color>>                 boardColor
)
{
    for (int row = 0; row < (int)board.size(); row++)
    {
        for (int col = 0; col < (int)board[row].size(); col++)
        {
            if (board[row][col] == Tetris::Game::BoardBlockType::BLOCK)
            {
                Canvas::drawBlock(canvas, col, row, boardColor[row][col]);
            }
        }
    }
}

void Canvas::drawBlock(
    ftxui::Canvas              &canvas,
    int                         x,
    int                         y,
    ftxui::Color                color,
    bool                        isCentered,
    bool                        isShadow,
    Tetris::Game::TetrominoType tetrominoType
)
{
    int offsetX = 0;

    if (isCentered && tetrominoType == Tetris::Game::TetrominoType::O)
    {
        offsetX = Canvas::stepX / 2;
    }

    if (isCentered && tetrominoType == Tetris::Game::TetrominoType::I)
    {
        y--;
        x--;
        offsetX = Canvas::stepX / 2;
    }

    canvas.DrawText(
        x * Canvas::stepX - offsetX,
        y * Canvas::stepY,
        isShadow ? Canvas::shadowBlockSymbol : Canvas::blockSymbol,
        [&](ftxui::Pixel &p) {
            p.foreground_color = color;

            if (!isShadow)
            {
                p.background_color = color;
            } else {
                p.background_color = Tetris::Renderer::CurrentTheme::backgroundColor;
            }
        }
    );
}

ftxui::Canvas Canvas::create(int width, int height)
{
    ftxui::Canvas canvas = ftxui::Canvas(width * Canvas::stepX, height * Canvas::stepY);

    for (int y = 0; y < width; y++)
    {
        for (int x = 0; x < height; x++)
        {
            canvas.DrawText(y * Canvas::stepX, x * Canvas::stepY, "  ", [&](ftxui::Pixel &p) {
                p.background_color = Tetris::Renderer::CurrentTheme::backgroundColor;
            });
        }
    }

    return canvas;
}
} // namespace Tetris::Renderer