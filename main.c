#include <stdlib.h> // rand
#include "nonsimple.h"
#include "bb3d.h"
#include "math.h" // fabs
#include "string.h" //memset

Camera camera;
vertex v[64]; // array of vertices
uint8_t neighbors[2][64][4];
int numv; // number of vertices

#define CAMERA_DISTANCE 2.2
#define CAMERA_MAGNIFICATION 75

#define MOVE_COLOR RGB(10, 200, 10)
#define ATTACK_COLOR RGB(255, 0, 0)

#define REACTION_TIME 7

uint16_t player_color[2] = { RGB(190, 150, 30), RGB(90, 130, 220) };

struct {
    uint8_t vertex_index; 
    uint8_t timer;
    uint16_t original_vertex_color;
} cursor[2]; // cursor 0 is current player location, cursor 1 is possible move location


uint8_t previously_played_vertex_index[2];
int8_t whose_turn;
uint8_t timer;
uint8_t reset_game_after_timer;
uint8_t hop_again;


inline uint16_t dim_color(uint16_t color, float z)
{
    if (z < 0.5)
        return color;
    if (round(2*z) > 7)
    {
        return (1 << 10)|(1 << 5)|1;
    }
    else
    {
        uint8_t zz = 8-round(2*z);
        uint8_t r,g,b;
        r = (color >> 10) & 31;
        g = (color >> 5) & 31;
        b = color & 31;
        r = (r*zz) / 8;
        g = (g*zz) / 8;
        b = (b*zz) / 8;
        return (r << 10)|(g << 5)|(b);
    }
}

inline void get_vertex(vertex *vv)
{
    float view[3]; // coordinates of the vertex within the camera's reference frame
    // you get this by matrix multiplication:
    matrix_multiply_vector0(view, camera.view_matrix, vv->world);
    if (view[2] <= 0.0f) // vertex is on/behind the camera
    {
        // put the vertex below the screen, so it won't get drawn.
        vv->ix = -10;
        vv->iy = 10000;
        vv->iz = view[2]; // allow for testing behindness
        return;
    }
    // apply perspective to the coordinates:
    view[0] *= camera.magnification / view[2]; 
    view[1] *= camera.magnification / view[2]; 
    vv->ix = SCREEN_W/2 + round(view[0]);
    vv->iy = SCREEN_H/2 + round(view[1]);
    vv->iz = view[2]; // allow for testing behindness
}

inline void draw_vertex(vertex *vv)
{
    // is it off screen?
    if (vv->ix < 0 || vv->ix >= SCREEN_W ||
        vv->iy < 0 || vv->iy >= SCREEN_H)
        return; // yep.

    // otherwise, draw it!
    superpixel[vv->iy][vv->ix] = dim_color(vv->color, vv->iz);

    // check if we should draw a player here, too:
    if (vv->player_index == 255)
        return; // no player
    
    uint16_t pc = dim_color(player_color[vv->player_index], vv->iz);

    switch (vv->power_level)
    {
    case 1:
        if (vv->ix > 0 && vv->iy > 0)
            superpixel[vv->iy-1][vv->ix-1] = pc;
        if (vv->ix < SCREEN_W-1 && vv->iy < SCREEN_H-1)
            superpixel[vv->iy+1][vv->ix+1] = pc;
        if (vv->iy > 0 && vv->ix < SCREEN_W-1)
            superpixel[vv->iy-1][vv->ix+1] = pc;
        if (vv->iy < SCREEN_H-1 && vv->ix > 0)
            superpixel[vv->iy+1][vv->ix-1] = pc;
        // NO BREAK
    case 0:
        if (vv->ix > 1)
            superpixel[vv->iy][vv->ix-2] = pc;
        if (vv->ix < SCREEN_W-2)
            superpixel[vv->iy][vv->ix+2] = pc;
        if (vv->iy > 1)
            superpixel[vv->iy-2][vv->ix] = pc;
        if (vv->iy < SCREEN_H-2)
            superpixel[vv->iy+2][vv->ix] = pc;
        break;
    }
}
void get_all_vertices()
{
    // determine the screen-coordinates of all vertices
    for (int i=0; i<numv; ++i)
    {
        get_vertex(&v[i]);
        #ifdef DEBUG
        message("v[%d].world=(%f, %f, %f), .image=(%d, %d, %f)\n", i, v[i].world[0], v[i].world[1], v[i].world[2], v[i].image[0], v[i].image[1], v[i].image_z);
        #endif
    }
}

void draw_all_vertices()
{
    // determine the screen-coordinates of all vertices
    for (int i=0; i<numv; ++i)
    {
        draw_vertex(&v[i]);
        #ifdef DEBUG
        message("v[%d].world=(%f, %f, %f), .image=(%d, %d, %f)\n", i, v[i].world[0], v[i].world[1], v[i].world[2], v[i].image[0], v[i].image[1], v[i].image_z);
        #endif
    }
}

void get_neighbors()
{
    memset(neighbors, 255, sizeof(neighbors)); // "zero" everybody
    for (int k=0; k<3; k++)
    for (int grid=0; grid<8; grid++)
    for (int next_grid=0; next_grid<8; next_grid++)
    {
//        if (k == 0 && grid == 0)
//        {
//            message("comparing:\n  v[%d].world=(%f, %f, %f)\n", (8*k+grid), v[(8*k+grid)].world[0], v[(8*k+grid)].world[1], v[(8*k+grid)].world[2]);
//            message("  v[%d].world=(%f, %f, %f)\n", (8+8*k+next_grid), v[(8+8*k+next_grid)].world[0], v[(8+8*k+next_grid)].world[1], v[(8+8*k+next_grid)].world[2]);
//        }
        if ((fabs(v[8*k+grid].x - 0.5f - v[8+8*k+next_grid].x) < 0.1) &&
            (fabs(v[8*k+grid].y - v[8+8*k+next_grid].y) < 0.1))
        {
            neighbors[0][8*k+grid][0] = 8+8*k+next_grid;
            neighbors[1][8+8*k+next_grid][0] = 8*k+grid;
        }
        else if ((fabs(v[8*k+grid].x + 0.5f - v[8+8*k+next_grid].x) < 0.1) &&
            (fabs(v[8*k+grid].y - v[8+8*k+next_grid].y) < 0.1))
        {
            neighbors[0][8*k+grid][2] = 8+8*k+next_grid;
            neighbors[1][8+8*k+next_grid][2] = 8*k+grid;
        }
        else if ((fabs(v[8*k+grid].x - v[8+8*k+next_grid].x) < 0.1) &&
            (fabs(v[8*k+grid].y - 0.5f - v[8+8*k+next_grid].y) < 0.1))
        {
            neighbors[0][8*k+grid][1] = 8+8*k+next_grid;
            neighbors[1][8+8*k+next_grid][1] = 8*k+grid;
        }
        else if ((fabs(v[8*k+grid].x - v[8+8*k+next_grid].x) < 0.1) &&
            (fabs(v[8*k+grid].y + 0.5f - v[8+8*k+next_grid].y) < 0.1))
        {
            neighbors[0][8*k+grid][3] = 8+8*k+next_grid;
            neighbors[1][8+8*k+next_grid][3] = 8*k+grid;
        }
    }
}

uint8_t check_forward_moves(uint8_t vindex)
{
    for (int i=0; i<4; ++i)
    {
        // check out neighbors:
        uint8_t n = neighbors[whose_turn][vindex][i];
        if ((n < 255) && // valid direction to go in
            (v[n].player_index == 255 || // spot is clear or...
             (v[n].player_index == (1-whose_turn) &&  // spot has enemy
              neighbors[whose_turn][n][i] < 255 &&   // but the next spot
              v[neighbors[whose_turn][n][i]].player_index == 255))  // is free! 
        )
            return n;
    }
    return 255;
}

uint8_t check_backward_moves(uint8_t vindex)
{
    for (int i=0; i<4; ++i)
    {
        // check out neighbors:
        uint8_t n = neighbors[1-whose_turn][vindex][i];
        if ((n < 255) && // valid direction to go in
            (v[n].player_index == 255 || // spot is clear or...
             (v[n].player_index == (1-whose_turn) &&  // spot has enemy
              neighbors[1-whose_turn][n][i] < 255 &&   // but the next spot
              v[neighbors[1-whose_turn][n][i]].player_index == 255))  // is free! 
        )
            return n;
    }
    return 255;
}

/*
void set_cursor(uint8_t c, uint8_t vertex_index)
{
    //uint8_t original_index = cursor[c].vertex_index;
    //superpixel[v[original_index].iy][v[original_index].ix] = cursor[c].original_vertex_color;
    cursor[c].vertex_index = vertex_index;
    //cursor[c].original_vertex_color = superpixel[v[vertex_index].iy][v[vertex_index].ix];
}
*/


int find_valid_move(uint8_t vindex)
{
    // check for a valid move...
    uint8_t mindex = -1;  // index of valid vertex to move to
    uint8_t last_to_check = (vindex - (1-2*whose_turn))%numv;
    // do something tricky here...
    vindex = last_to_check;
    do
    {
        // increment/decrement vindex at the beginning
        vindex += 1-2*whose_turn; 
        vindex %= numv;

        // see if vindex has a playable move.
        if (v[vindex].player_index == whose_turn)
        {    // good start, it's the player's vertex
            // check forward moves first:
            mindex = check_forward_moves(vindex);
            if (mindex < 255)
                break;
            if (v[vindex].power_level)
            {
                mindex = check_backward_moves(vindex);
                if (mindex < 255)
                    break;
            }
        }
    } while (vindex != last_to_check);

    // did we find any valid moves?
    if (mindex == (uint8_t)(-1))
    {
        // nope.
        return 0;
    }
    else // yes, there was a valid move.
    {
        cursor[0].vertex_index = vindex;
        cursor[1].vertex_index = mindex;
        cursor[0].timer = 0;
        cursor[1].timer = 128;
        return 1;
    }
}

void game_init()
{
    // setup the board with some random vertices
    numv = 0; 
    for (int k=0; k<4; ++k)
    for (int j=0; j<4; ++j)
    for (int i=0; i<4; ++i)
    {
        if (  ((k%2) && ((i%2)==(j%2))) || (!(k%2) && ((i%2) != (j%2)))  )
        {
            v[numv++] = (vertex) { 
                .world = { -0.75 + 0.5*i, -0.75 + 0.5*j, -1.2 + 0.8*k },
                .player_index = 255,
                .power_level = 0,
                .color = RGB(255,255,255),
            };
            message("v[%d].(x,y,z) = (%f, %f, %f)\n", (numv-1), v[numv-1].world[0], v[numv-1].world[1], v[numv-1].world[2]);
        }
    }
    get_neighbors();

    // reset player pieces:
    for (int i=0; i<8; ++i)
    {
        v[i].player_index = 0;
        v[numv-i-1].player_index = 1;
    }
    //v[0].power_level = 1;
    //v[numv-1].power_level = 1;
    previously_played_vertex_index[0] = 0;
    previously_played_vertex_index[1] = numv-1;


    // setup the camera
    camera = (Camera) {
        .viewer = {0,0,-CAMERA_DISTANCE},
        .viewee = {0,0,0},
        .down = {0,1,0},
        .magnification = CAMERA_MAGNIFICATION
    };
    // get the view of the camera:
    get_view(&camera);
    #ifdef DEBUG
    message("cam matrix: ");
    for (int i=0; i<12; ++i)
        message("%f, ", camera.view_matrix[i]);
    message("\n");
    #endif

    // get the vertices' screen positions:
    clear();
    get_all_vertices();
    draw_all_vertices();

    #ifdef DEBUG
    for (int i=0; i<numv; ++i)
    {
        message("v[%d].world=(%f, %f, %f), .image=(%d, %d, %f)\n", i, v[i].world[0], v[i].world[1], v[i].world[2], v[i].image[0], v[i].image[1], v[i].image_z);
    }
    #endif

    // give player 0 to start
    whose_turn = 0;
    find_valid_move(previously_played_vertex_index[whose_turn]); // start from vertex index 0
    //if (!find_valid_move(0));
    //    die();
   
    // important to reset this!
    reset_game_after_timer = 0;
    hop_again = 0;
}

void game_reset()
{
    timer = REACTION_TIME*3;
    reset_game_after_timer = 1;
}

void draw_cursor_square_vertex(vertex *vv, uint16_t color)
{
    //uint8_t index = cursor[c].vertex_index;
    if (vv->ix < 0 || vv->ix >= SCREEN_W ||
        vv->iy < 0 || vv->iy >= SCREEN_H)
        return;
   
    if (vv->iy > 2)
    {
        if (vv->ix > 2)
            superpixel[vv->iy-3][vv->ix-3] = color;
        if (vv->ix < SCREEN_W-3)
            superpixel[vv->iy-3][vv->ix+3] = color;
    }
    if (vv->iy < SCREEN_H-3)
    {
        if (vv->ix > 2)
            superpixel[vv->iy+3][vv->ix-3] = color;
        if (vv->ix < SCREEN_W-3)
            superpixel[vv->iy+3][vv->ix+3] = color;
    }
}

void draw_cursor_cross_vertex(vertex *vv, uint16_t color)
{
    //uint8_t index = cursor[c].vertex_index;
    
    if (vv->ix < 0 || vv->ix >= SCREEN_W ||
        vv->iy < 0 || vv->iy >= SCREEN_H)
        return;
    
    if (vv->iy > 3)
        superpixel[vv->iy-4][vv->ix] = color;
    if (vv->iy < SCREEN_H-4)
        superpixel[vv->iy+4][vv->ix] = color;
    if (vv->ix > 3)
        superpixel[vv->iy][vv->ix-4] = color;
    if (vv->ix < SCREEN_W-4)
        superpixel[vv->iy][vv->ix+4] = color;
}


void game_frame()
{
    kbd_emulate_gamepad();

    // move camera to arrow keys (or d-pad):
    const float delta = 0.1;
    int need_new_view = 0;
    if (GAMEPAD_PRESSED(0, select))
    {
        camera = (Camera) {
            .viewer = {0,0,(-1+2*whose_turn)*CAMERA_DISTANCE},
            .viewee = {0,0,0},
            .down = {0,1,0},
            .magnification = CAMERA_MAGNIFICATION
        };
        need_new_view = 1;
    }
    else
    {
        if (GAMEPAD_PRESSED(0, left)) 
        {
            for (int i=0; i<3; ++i)
                camera.viewer[i] -= delta*camera.right[i];
            need_new_view = 1;
        }
        else if (GAMEPAD_PRESSED(0, right)) 
        {
            for (int i=0; i<3; ++i)
                camera.viewer[i] += delta*camera.right[i];
            need_new_view = 1;
        }
        if (GAMEPAD_PRESSED(0, down))
        {
            for (int i=0; i<3; ++i)
                camera.viewer[i] += delta*camera.down[i];
            need_new_view = 1;
        }
        else if (GAMEPAD_PRESSED(0, up))
        {
            for (int i=0; i<3; ++i)
                camera.viewer[i] -= delta*camera.down[i];
            need_new_view = 1;
        }
    }

    if (need_new_view)
    {
        // fix the camera at four units away from the origin
        normalize(camera.viewer, camera.viewer);
        for (int i=0; i<3; ++i)
            camera.viewer[i] *= CAMERA_DISTANCE;
        // need to still update the view matrix of the camera,
        get_view(&camera);
        // clear screen
        clear();
        // and then apply the matrix to all vertex positions:
        get_all_vertices();
        // then draw
        draw_all_vertices();
        // draw cursors
        cursor[0].timer = 0;
        draw_cursor_square_vertex(&v[cursor[0].vertex_index], RGB(255,255,255)); //player_color[whose_turn]);

        cursor[1].timer = 128;
        if ((v[cursor[1].vertex_index].player_index != 255) &&
            (v[cursor[1].vertex_index].player_index != whose_turn))
            // enemy is being targeted!
            draw_cursor_cross_vertex(&v[cursor[1].vertex_index], ATTACK_COLOR);
        else
            draw_cursor_cross_vertex(&v[cursor[1].vertex_index], MOVE_COLOR);
    }
    else
    {
        // update the cursors
        cursor[0].timer += 2;
        switch (cursor[0].timer)
        {
        case 0:
        case 128:
            draw_cursor_square_vertex(&v[cursor[0].vertex_index], player_color[whose_turn]);
            break;
        case 64:
            draw_cursor_square_vertex(&v[cursor[0].vertex_index], RGB(255,255,255));
            break;
        case 192:
            draw_cursor_square_vertex(&v[cursor[0].vertex_index], 0);
        }
        cursor[1].timer += 2;
        switch (cursor[1].timer)
        {
        case 0:
        case 128:
            if ((v[cursor[1].vertex_index].player_index != 255) &&
                (v[cursor[1].vertex_index].player_index != whose_turn))
                draw_cursor_cross_vertex(&v[cursor[1].vertex_index], ATTACK_COLOR);
            else
                draw_cursor_cross_vertex(&v[cursor[1].vertex_index], MOVE_COLOR);
            break;
        case 64:
            draw_cursor_cross_vertex(&v[cursor[1].vertex_index], RGB(255,255,255));
            break;
        case 192:
            draw_cursor_cross_vertex(&v[cursor[1].vertex_index], 0);
        }
    }

    if (timer)
    {
        if (vga_frame % 2 == 0)
        {
            timer--;
            if (timer == 0 && reset_game_after_timer)
                return game_init(); 
        }
        return;
    }
    
    if (GAMEPAD_PRESSED(0, start))
    {
        if (GAMEPAD_PRESSED(0, R) && GAMEPAD_PRESSED(0, L))
            return game_reset();
        // execute move
        uint8_t old_spot = cursor[0].vertex_index;
        uint8_t new_spot;
        if (v[cursor[1].vertex_index].player_index < 255)
        {   // hop over the enemy
            uint8_t jump_over = cursor[1].vertex_index;
            v[jump_over].player_index = 255; // kill player on jump_over
            // need to find the direction of hopping...
            // i.e. the dir:
            uint8_t dir = 0;
            message("hoping to hop over %d:\n  ", (int)jump_over);
            for (; dir<4; dir++)
                if (neighbors[whose_turn][old_spot][dir] == jump_over)
                    break;
            if (dir > 3)
            {
                for (; dir<8; dir++)
                    if (neighbors[1-whose_turn][old_spot][dir-4] == jump_over)
                        break;

                if (dir > 7)
                {
                    message("weird!  expected jump_over to be forward or back of old_spot\n");
                    new_spot = jump_over; // somehow teleport there.
                }
                else
                    // it was a backwards move:  continue in same direction from jump_over:
                    new_spot = neighbors[1-whose_turn][jump_over][dir-4];
            }
            else
                // standard forward play:  continue from jump_over:
                new_spot = neighbors[whose_turn][jump_over][dir];
            message("got direction %d, new spot %d\n", (int)dir, (int)new_spot);

            // check to see if we made it all the way:
            if (whose_turn)
            {
                if (new_spot < 8)
                    v[old_spot].power_level = 1;
            }
            else
            {
                if (new_spot >= numv-8)
                    v[old_spot].power_level = 1;
            }
            hop_again = 0; // here is where we should check if it's possible to hop again.
            // if not, hop_again = 0, if so, hop_again = 1.
        }
        else
        {
            new_spot = cursor[1].vertex_index;
            if (whose_turn)
            {
                if (new_spot < 8)
                    v[old_spot].power_level = 1;
            }
            else
            {
                if (new_spot >= numv-8)
                    v[old_spot].power_level = 1;
            }
        }
        // now do the moving from old to new:
        v[new_spot].power_level = v[old_spot].power_level;
        v[new_spot].player_index = whose_turn;
        v[old_spot].player_index = 255; // remove player from vindex


        // if no extra hopping, check that the other team has a move before giving them an option
        if (!hop_again)
        {
            previously_played_vertex_index[whose_turn] = new_spot;

            whose_turn = 1 - whose_turn;
            uint8_t valid_move[2] = {1, 1};
            while (valid_move[whose_turn])
            {
                if (find_valid_move(previously_played_vertex_index[whose_turn]))
                    // will set up the cursors for this player if so!
                    break;
                else
                {
                    valid_move[whose_turn] = 0;
                    whose_turn = 1 - whose_turn;
                }
            }
            // no one has valid moves:  reset the game
            if (!valid_move[whose_turn])
                game_reset();
        }    
        timer = 3*REACTION_TIME;
        clear();
        draw_all_vertices();
    }

    if (GAMEPAD_PRESSED(0, L) || GAMEPAD_PRESSED(0, R))
    {
        if (GAMEPAD_PRESSED(0, L) && GAMEPAD_PRESSED(0, R))
            return;
        // find another piece to consider its movement
        uint8_t search_dir = 1;
        if (GAMEPAD_PRESSED(0, L))
            search_dir = -1;
        
        // search for the next object which can be played...
        uint8_t vindex = (cursor[0].vertex_index + search_dir)%numv;
        uint8_t mindex = 255; // place where vindex can move to
        while (mindex == 255)
        {
            // skip over any non-friendly vertices:
            while (v[vindex].player_index != whose_turn)
                vindex = (vindex + search_dir)%numv;
            
            // now search for a valid move...
            mindex = check_forward_moves(vindex);
            if (mindex < 255)
                break;
            if (v[vindex].power_level)
            {
                mindex = check_backward_moves(vindex);
                if (mindex < 255)
                    break;
            }
            // otherwise didn't find anything, keep searching
            vindex = (vindex + search_dir)%numv; 
        }
        // we are guaranteed to find a stopping point, because we check
        // for a move at the beginning of a turn.

        // reset the superpixels:
        message("found vindex %d, with v[vindex].player_index = %d\n", (int)vindex, (int)v[vindex].player_index);
        cursor[0].vertex_index = vindex;
        cursor[1].vertex_index = mindex;
        clear();
        draw_all_vertices();
        cursor[0].timer = 62;
        cursor[1].timer = 190;
        timer = REACTION_TIME;
   
        message("neighbors of vertex %d:\n", vindex);
        for (int i=0; i<4; ++i)
            message("  %d\n", (int) neighbors[whose_turn][vindex][i]);
    }
   
    // A goes forwards, B goes backwards
    // X goes backwards, Y goes forwards
    if (GAMEPAD_PRESSED(0, A) || GAMEPAD_PRESSED(0, Y) ||
        GAMEPAD_PRESSED(0, B) || GAMEPAD_PRESSED(0, X))
    {
        if ((GAMEPAD_PRESSED(0, A) || GAMEPAD_PRESSED(0, Y)) &&
            (GAMEPAD_PRESSED(0, B) || GAMEPAD_PRESSED(0, X)))
            return;

        uint8_t search_dir = 1;
        if (GAMEPAD_PRESSED(0, B) || GAMEPAD_PRESSED(0, X))
            search_dir = -1;

        search_dir *= (uint8_t)(1 - 2*whose_turn);

        // locate any other possible places to move to,
        // but first find the index dir in direction-space 
        // which got us from cursor[0] to cursor[1]
        uint8_t vindex = cursor[0].vertex_index;
        uint8_t mindex = cursor[1].vertex_index;
        uint8_t dir = 0;
        for (; dir<4; dir++)
            if (neighbors[whose_turn][vindex][dir] == mindex)
                break;
        if (dir > 3)
        {
            for (; dir<8; dir++)
                if (neighbors[1-whose_turn][vindex][dir-4] == mindex)
                    break;

            if (dir > 7)
                message("weird!  expected mindex to be forward or back of vindex\n");
        }
        else
            message("\nfound dir = %d:\n  ", dir);

        mindex = 255;
        while (1)
        {
            message(" first dir = %d", (int) dir);
            if (v[vindex].power_level)
                dir = (dir + search_dir) % 8;
            else
                dir = (dir + search_dir) % 4;
            message(" after dir = %d\n", (int) dir);

            if (dir < 4)
            {
                // check out neighbors:
                uint8_t n = neighbors[whose_turn][vindex][dir];
                if ((n < 255) && // valid direction to go in
                    (v[n].player_index == 255 || // spot is clear or...
                     (v[n].player_index == (1-whose_turn) &&  // spot has enemy
                      neighbors[whose_turn][n][dir] < 255 &&   // but the next spot
                      v[neighbors[whose_turn][n][dir]].player_index == 255))  // is free! 
                )
                {
                    mindex = n;
                    break;
                }
            }
            else
            {
                // check out backward neighbors:
                uint8_t n = neighbors[1-whose_turn][vindex][dir-4];
                if ((n < 255) && // valid direction to go in
                    (v[n].player_index == 255 || // spot is clear or...
                     (v[n].player_index == (1-whose_turn) &&  // spot has enemy
                      neighbors[1-whose_turn][n][dir-4] < 255 &&   // but the next spot
                      v[neighbors[1-whose_turn][n][dir-4]].player_index == 255))  // is free! 
                )
                {
                    mindex = n;
                    break;
                }
            }
        }
        // we are guaranteed to find a stopping point, because we check
        // for a move at the beginning of a turn.

        // now reset the original vertex color
        cursor[1].vertex_index = mindex;
        clear();
        draw_all_vertices();
        cursor[0].timer = 126;
        draw_cursor_square_vertex(&v[cursor[0].vertex_index], player_color[whose_turn]);
        cursor[1].timer = 254;
        timer = REACTION_TIME;
    }
}
