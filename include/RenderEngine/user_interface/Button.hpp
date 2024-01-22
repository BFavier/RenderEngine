#pragma once

namespace RenderEngine
{
    //Describes the status of a button
    struct Button
    {
        bool down = false; //True if the key is pressed
        bool was_pressed = false; //True if the button was pressed at the last update
        bool was_released = false; //True if the button was released at the last update
    };
}
