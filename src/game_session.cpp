//  Pingus - A free Lemmings clone
//  Copyright (C) 2000 Ingo Ruhnke <grumbel@gmx.de>
//
//  This program is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
//  
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//  
//  You should have received a copy of the GNU General Public License
//  along with this program.  If not, see <http://www.gnu.org/licenses/>.

#include <iostream>
#include "screen/screen_manager.hpp"
#include "server.hpp"
#include "gui/gui_manager.hpp"
#include "display/display.hpp"

#include "components/button_panel.hpp"
#include "components/pingus_counter.hpp"
#include "components/time_display.hpp"
#include "components/smallmap.hpp"
#include "components/playfield.hpp"
#include "input/event.hpp"
#include "sound/sound.hpp"
#include "math.hpp"
#include "game_session.hpp"
#include "resource.hpp"
#include "pingu_holder.hpp"
#include "world.hpp"
#include "result_screen.hpp"
#include "savegame_manager.hpp"
#include "globals.hpp"
#include "debug.hpp"

GameSession::GameSession (const PingusLevel& arg_plf, bool arg_show_result_screen)
  : plf(arg_plf),
    show_result_screen(arg_show_result_screen),
    is_finished  (false),
    button_panel (0),
    pcounter     (0),
    playfield    (0),
    time_display (0),
    small_map    (0),
    pause(false),
    fast_forward(false)
{
  server = std::auto_ptr<Server>(new Server(plf, true));

  // the world is initially on time
  world_delay = 0;

  pout(PINGUS_DEBUG_LOADING) << "GameSession" << std::endl;

  // -- Client stuff
  
  // These object will get deleted by the gui_manager
  button_panel = new ButtonPanel(this, 2, Display::get_height()/2);

  int world_width  = server->get_world()->get_width();
  int world_height = server->get_world()->get_height();

  playfield    = new Playfield(get_server(), this,
                               Rect(Vector2i(Math::max((Display::get_width()  - world_width)/2,  0),
                                             Math::max((Display::get_height() - world_height)/2, 0)), 
                                    Size(Math::min(Display::get_width(),  world_width),
                                         Math::min(Display::get_height(), world_height))));

  pcounter     = new PingusCounter(get_server());
  small_map    = new SmallMap(get_server(), playfield);
  time_display = new TimeDisplay(this);

  gui_manager->add(playfield);
  gui_manager->add(button_panel);
  gui_manager->add(pcounter);
  gui_manager->add(small_map);
  gui_manager->add(time_display);

  armageddon_button = new ArmageddonButton(get_server(), Display::get_width() - 40, Display::get_height() - 62);
  forward_button    = new ForwardButton(this, Display::get_width() - 40 * 2, Display::get_height() - 62);
  pause_button      = new PauseButton(this, Display::get_width() - 40 * 3, Display::get_height() - 62);

  gui_manager->add(armageddon_button);
  gui_manager->add(forward_button);
  gui_manager->add(pause_button);
}

GameSession::~GameSession ()
{
}

void
GameSession::update_server(float delta)
{
  // FIXME: Timing code could need another rewrite...
  if (server->is_finished())
    {
      //ScreenManager::instance()->pop_screen();
      PinguHolder* pingu_holder = server->get_world()->get_pingus();
      Result result;

      result.plf    = server->get_plf();

      result.saved  = pingu_holder->get_number_of_exited();
      result.killed = pingu_holder->get_number_of_killed();
      result.total  = server->get_plf().get_number_of_pingus();

      result.needed = server->get_plf().get_number_to_save();

      result.max_time  = server->get_plf().get_time();
      result.used_time = server->get_time();

      { // Write the savegame
        Savegame savegame(result.plf.get_resname(),
                          (result.saved >= result.needed) ? Savegame::FINISHED : Savegame::ACCESSIBLE,
                          result.used_time,
                          result.saved);
        SavegameManager::instance()->store(savegame);
      }

      if (show_result_screen)
        ScreenManager::instance()->replace_screen(new ResultScreen(result));
      else
        ScreenManager::instance()->pop_screen();

    }
  else
    {
      // how much time we have to account for while doing world updates
      int time_passed = int(delta * 1000) + world_delay;
      // how much time each world update represents
      int update_time = game_speed;

      // update the world (and the objects in it) in constant steps to account
      // for the time the previous frame took

      // invariant: world_updates - the number of times the world
      // has been updated during this frame
      int world_updates = 0;

      while ((world_updates+1)*update_time <= time_passed)
        {
          if (!pause)
            {
              if (fast_forward)
                {
                  for (int i = 0; i < fast_forward_time_scale; ++i)
                    server->update();
                }
              else
                {
                  server->update();
                }
            }

          world_updates++;
        }
      // save how far behind is the world compared to the actual time
      // so that we can account for that while updating in the next frame
      world_delay = time_passed - (world_updates*update_time);
    }
}

void
GameSession::draw_background (DrawingContext& gc)
{
  Rect rect = playfield->get_rect();
  
  if (rect != Rect(Vector2i(0,0), Size(Display::get_width(), Display::get_height())))
    { // Draw a black border around the playfield when the playfield is smaller then the screen
      Color border_color(0, 0, 0);
      // top
      gc.draw_fillrect(0, 0, Display::get_width(), rect.top,
                       border_color);
      // bottom
      gc.draw_fillrect(0, rect.bottom, Display::get_width(), Display::get_height(),
                       border_color);
      // left
      gc.draw_fillrect(0, rect.top, rect.left, rect.bottom,
                       border_color);
      // right
      gc.draw_fillrect(rect.right, rect.top, Display::get_width(), rect.bottom,
                       border_color);
    }
}

void
GameSession::update(float delta)
{
  update_server(delta);
  GUIScreen::update(delta);
}

void
GameSession::update(const Input::Event& event)
{
  GUIScreen::update(event);

  //std::cout << "Events: " << event.get_type () << std::endl;

  switch (event.type)
    {
      case Input::BUTTON_EVENT_TYPE:
        {
          const Input::ButtonEvent& ev = event.button;

          if (ev.state == Input::BUTTON_PRESSED)
            {
              if (ev.name >= Input::ACTION_1_BUTTON && ev.name <= Input::ACTION_10_BUTTON)
                {
                  button_panel->set_button(ev.name - Input::ACTION_1_BUTTON);
                }
              else if (ev.name == Input::ACTION_DOWN_BUTTON)
                {
                  button_panel->next_action();
                }
              else if (ev.name == Input::ACTION_UP_BUTTON)
                {
                  button_panel->previous_action();
                }
            }
        }
        break;

      case Input::POINTER_EVENT_TYPE:
        // Ignore, is handled in GUIScreen
        break;

      case Input::AXIS_EVENT_TYPE:
        // ???
        process_axis_event (event.axis);
        break;

      case Input::SCROLLER_EVENT_TYPE:
        process_scroll_event(event.scroll);
        break;

      case Input::KEYBOARD_EVENT_TYPE:
        break;

      default:
        // unhandled event
        std::cout << "GameSession::process_events (): unhandled event: " << event.type << std::endl;
        break;
    }
}

void
GameSession::process_scroll_event (const Input::ScrollEvent& ev)
{
  playfield->scroll(static_cast<int>(-ev.x_delta),
                    static_cast<int>(-ev.y_delta));
}

void
GameSession::process_axis_event (const Input::AxisEvent& event)
{
  // std::cout << "GameSession::process_axis_event ()" << std::endl;
  UNUSED_ARG(event);
}

void
GameSession::do_restart()
{
  server->send_finish_event();
}

bool
GameSession::finished()
{
  return is_finished;
}

void
GameSession::set_finished()
{
  is_finished = true;
  server->send_finish_event();
}

void
GameSession:: on_escape_press ()
{
  server->send_finish_event();
}

void
GameSession:: on_pause_press ()
{
  pause = !pause;
}

void
GameSession::on_fast_forward_press ()
{
  fast_forward = !fast_forward;
}

void
GameSession::on_armageddon_press ()
{
  server->send_armageddon_event();
}

void
GameSession::on_action_axis_move (float move)
{
  if (move > 0)
    button_panel->next_action ();
  else if (move < 0)
    button_panel->previous_action ();
}

void
GameSession::on_startup ()
{
  is_finished = false;

  if (maintainer_mode)
    std::cout << "Starting Music: " << server->get_plf().get_music() << std::endl;

  if (server->get_plf().get_music() == "none")
    {
      Sound::PingusSound::stop_music();
    }
  else
    {
      Sound::PingusSound::play_music(server->get_plf().get_music());
    }

  if (verbose)
    std::cout << "GameSession: Entering main_loop. Startup time: "
	      << SDL_GetTicks() << " msec." << std::endl;
}

void
GameSession::on_shutdown ()
{
}

Actions::ActionName
GameSession::get_action_name() const
{
  return button_panel->get_action_name();
}

void
GameSession::set_fast_forward(bool value)
{
  fast_forward = value;
}

bool
GameSession::get_fast_forward() const
{
  return fast_forward;
}

void
GameSession::set_pause(bool value)
{
  pause = value;
}

bool
GameSession::get_pause() const
{
  return pause;
}

void
GameSession::resize(const Size& size)
{
  GUIScreen::resize(size);

  int world_width  = server->get_world()->get_width();
  int world_height = server->get_world()->get_height();
  
  playfield->set_rect(Rect(Vector2i(Math::max((Display::get_width()  - world_width)/2,  0),
                                    Math::max((Display::get_height() - world_height)/2, 0)), 
                           Size(Math::min(Display::get_width(),  world_width),
                                Math::min(Display::get_height(), world_height))));

  armageddon_button->set_rect(Rect(Vector2i(Display::get_width() - 40, Display::get_height() - 62),
                                   Size(38, 60)));
  forward_button->set_rect(Rect(Vector2i(Display::get_width() - 40*2, Display::get_height() - 62),
                                Size(38, 60)));
  pause_button->set_rect(Rect(Vector2i(Display::get_width() - 40*3, Display::get_height() - 62),
                                Size(38, 60)));
}

/* EOF */
