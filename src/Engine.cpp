/*
 *  MonolithEngine.cpp
 *  Monolith1
 *
 *  Created by Niblit on 2011-02-07.
 *  Copyright 2011 __MyCompanyName__. All rights reserved.
 *
 */

#include "Engine.h"
#include "SystemContext.h"
#include "InputHandler.h"
#include "Updater.h"

#include "Camera/ICamera.h"
#include "Zone/IZone.h"

#include "System/Audio.h"
#include "System/System.h"

#include "EventHandler/EventHandler.h"
#include "Events/EventFuncFwd.h"
#include "Events/PauseEvent.h"
#include "Events/QuitEvent.h"
#include "Events/ApplicationEvent.h"
#include "Events/SurfaceChangedEvent.h"
#include "Events/ActivatedEvent.h"
#include "Events/TimeScaleEvent.h"

#include "Rendering/RendererSokol.h"

#include "Math/Vector.h"
#include "Math/Quad.h"

using namespace mono;

Engine::Engine(System::IWindow* window, ICamera* camera, SystemContext* system_context, EventHandler* event_handler)
    : m_window(window)
    , m_camera(camera)
    , m_system_context(system_context)
    , m_event_handler(event_handler)
{
    using namespace std::placeholders;

    const event::PauseEventFunc pause_func = std::bind(&Engine::OnPause, this, _1);
    const event::QuitEventFunc quit_func = std::bind(&Engine::OnQuit, this, _1);
    const event::ApplicationEventFunc app_func = std::bind(&Engine::OnApplication, this, _1);
    const event::ActivatedEventFunc activated_func = std::bind(&Engine::OnActivated, this, _1);
    const event::TimeScaleEventFunc time_scale_func = std::bind(&Engine::OnTimeScale, this, _1);

    m_pause_token = m_event_handler->AddListener(pause_func);
    m_quit_token = m_event_handler->AddListener(quit_func);
    m_application_token = m_event_handler->AddListener(app_func);
    m_activated_token = m_event_handler->AddListener(activated_func);
    m_time_scale_token = m_event_handler->AddListener(time_scale_func);
}

Engine::~Engine()
{
    m_event_handler->RemoveListener(m_pause_token);
    m_event_handler->RemoveListener(m_quit_token);
    m_event_handler->RemoveListener(m_application_token);
    m_event_handler->RemoveListener(m_activated_token);
    m_event_handler->RemoveListener(m_time_scale_token);
}

int Engine::Run(IZone* zone)
{
    RendererSokol renderer;

    const ScreenToWorldFunc screen_to_world_func = [this](float& x, float& y) {
        const math::Vector world = m_camera->ScreenToWorld(math::Vector(x, y));
        x = world.x;
        y = world.y;
    };

    InputHandler input_handler(screen_to_world_func, m_event_handler);
    UpdateContext update_context = { 0, 0, 0, 0.0f };
    Updater updater;

    zone->OnLoad(m_camera, &renderer);

    uint32_t last_time = System::GetMilliseconds();

    while(!m_quit)
    {
        // When exiting the application on iOS the lastTime variable
        // will be from when you exited, and then when you resume
        // the app the calculated delta will be huge and screw
        // everything up, thats why we need to update it here.
        if(m_update_last_time)
        {
            last_time = System::GetMilliseconds();
            m_update_last_time = false;
        }

        const uint32_t before_time = System::GetMilliseconds();
        const uint32_t delta_ms = std::clamp(uint32_t((before_time - last_time) * m_time_scale), 1u, std::numeric_limits<uint32_t>::max());
        update_context.timestamp += delta_ms;

        const System::Size& size = m_window->Size();
        const math::Vector window_size(size.width, size.height);
        m_camera->SetWindowSize(window_size);

        const System::Size drawable_size = m_window->DrawableSize();
        const math::Vector drawable_size_vec(drawable_size.width, drawable_size.height);
        renderer.SetWindowSize(window_size);
        renderer.SetDrawableSize(drawable_size_vec);

        const math::Quad& viewport = m_camera->GetViewport();
        const math::Quad camera_quad(viewport.mA, viewport.mA + viewport.mB);
        renderer.SetViewport(camera_quad);

        // Handle input events
        System::ProcessSystemEvents(&input_handler);

        audio::MixSounds();

        if(!m_pause)
        {
            update_context.frame_count++;
            update_context.delta_ms = delta_ms;
            update_context.delta_s = float(delta_ms) / 1000.0f;

            renderer.SetDeltaAndTimestamp(update_context.delta_ms, update_context.delta_s, update_context.timestamp);

            m_window->MakeCurrent();
            m_system_context->Update(update_context);

            // Update all the stuff...
            zone->Accept(updater);
            updater.AddUpdatable(m_camera);
            updater.Update(update_context);

            // Draw...
            zone->Accept(renderer);
            renderer.DrawFrame();

            m_window->SwapBuffers();

            zone->PostUpdate();
            m_system_context->Sync();
        }

        last_time = before_time;

        // Sleep for a millisecond, this highly reduces fps
        System::Sleep(1);
    }

    // Remove possible follow entity and unload the zone
    const int exit_code = zone->OnUnload();

    // Reset the quit, pause and m_update_last_time flag for when you want
    // to reuse the engine for another zone.
    m_quit = false;
    m_pause = false;
    m_update_last_time = false;
    m_time_scale = 1.0f;

    return exit_code;
}

mono::EventResult Engine::OnPause(const event::PauseEvent& event)
{
    m_pause = event.pause;
    return mono::EventResult::HANDLED;
}

mono::EventResult Engine::OnQuit(const event::QuitEvent&)
{
    m_quit = true;
    return mono::EventResult::PASS_ON;
}

mono::EventResult Engine::OnApplication(const event::ApplicationEvent& event)
{
    if(event.state == event::ApplicationState::ENTER_BACKGROUND)
    {
        m_pause = true;
    }
    else if(event.state == event::ApplicationState::ENTER_FOREGROUND)
    {
        m_pause = false;
        m_update_last_time = true;
    }

    return mono::EventResult::PASS_ON;
}

mono::EventResult Engine::OnActivated(const event::ActivatedEvent& event)
{
    //m_window->Activated(event.gain);
    return mono::EventResult::PASS_ON;
}

mono::EventResult Engine::OnTimeScale(const event::TimeScaleEvent& event)
{
    m_time_scale = event.time_scale;
    return mono::EventResult::PASS_ON;
}
