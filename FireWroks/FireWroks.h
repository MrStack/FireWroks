#pragma once

#include "resource.h"

#include <windows.h>
#include <d2d1.h>
#include <memory>
#include <cmath>
#include <thread>
#include <functional>
#include <string>
#include <vector>
#include <random>
#include <numeric>
#include <tuple>

#define PI 3.1415926535f

template <typename T>
void SafeRelease(T** ppT)
{
    if (*ppT)
    {
        (*ppT)->Release();
        *ppT = NULL;
    }
}

class Drawer
{
public:
    Drawer(HWND hwnd) :pD2DFactory{}, hr{}, hWnd{ hwnd }, pRT{}, pEdgeBrush{}, rc{}
	{		
		hr = D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, &pD2DFactory);

        //GetClientRect(hWnd, &rc);

        //// Create a Direct2D render target        
        //hr = pD2DFactory->CreateHwndRenderTarget(
        //    D2D1::RenderTargetProperties(),
        //    D2D1::HwndRenderTargetProperties(hWnd, D2D1::SizeU(
        //            rc.right - rc.left,
        //            rc.bottom - rc.top)), &pRT);
	}
    ~Drawer()
    {
        SafeRelease(&pRT);
        SafeRelease(&pEdgeBrush);
        SafeRelease(&pD2DFactory);
    }

    HRESULT CreateDeviceResources(void)
    {
        HRESULT hr = S_OK;

        if (!pRT)
        {
            RECT rc;
            GetClientRect(hWnd, &rc);

            D2D1_SIZE_U size = D2D1::SizeU(
                rc.right - rc.left,
                rc.bottom - rc.top
            );

            // Create a Direct2D render target
            hr = pD2DFactory->CreateHwndRenderTarget(
                D2D1::RenderTargetProperties(),
                D2D1::HwndRenderTargetProperties(hWnd, size),
                &pRT
            );

            if (SUCCEEDED(hr))
            {
                // Create a red brush.
                hr = pRT->CreateSolidColorBrush(
                    D2D1::ColorF(D2D1::ColorF::Red),
                    &pEdgeBrush
                );
            }
            if (SUCCEEDED(hr))
            {
                hr = pRT->CreateSolidColorBrush(
                    D2D1::ColorF(D2D1::ColorF::Red),
                    &pFaceBrush
                );
            }
        }

        return hr;
    }

    void CreateSolidBrush(UINT32 color, float alpha = 1.0f)
    {        
        if (SUCCEEDED(hr))
        {
            pRT->CreateSolidColorBrush(D2D1::ColorF(color), &pEdgeBrush);
            pEdgeBrush->SetOpacity(alpha);
        }
    }
    void DiscardDeviceResources(void)
    {
        SafeRelease(&pRT);
        SafeRelease(&pEdgeBrush);
    }
    void DrawRect(void)
    {
        static float x{};
        static float y{};
        x++;

        HRESULT hr;

        hr = CreateDeviceResources();
        if (SUCCEEDED(hr) && !(pRT->CheckWindowState() & D2D1_WINDOW_STATE_OCCLUDED))
        {
            auto rect = D2D1::RectF(
                rc.left + 10.f,
                rc.top + 10.f,
                rc.right - 10.f,
                rc.bottom - 10.f);


            pRT->BeginDraw();

            pRT->SetTransform(D2D1::Matrix3x2F::Identity());
            pRT->Clear(D2D1::ColorF(D2D1::ColorF::Black));

            pRT->SetTransform(D2D1::Matrix3x2F::Translation(x, x));

            pRT->FillRectangle(rect, pFaceBrush);
            pRT->DrawRectangle(rect, pEdgeBrush);

            pRT->EndDraw();

            if (hr == D2DERR_RECREATE_TARGET)
            {
                hr = S_OK;
                DiscardDeviceResources();
            }
        }
        InvalidateRect(hWnd, NULL, FALSE);
    }

    void BeginDraw(void)
    {
        pRT->BeginDraw();
    }

    void EndDraw(void)
    {
        hr = pRT->EndDraw();
    }

    void Clear(const D2D_COLOR_F& color)
    {
        BeginDraw();
        pRT->SetTransform(D2D1::Matrix3x2F::Identity());
        pRT->Clear(color);
        EndDraw();
    }

    void Clear(void)
    {
        HRESULT hr;

        hr = CreateDeviceResources();
        if (SUCCEEDED(hr) && !(pRT->CheckWindowState() & D2D1_WINDOW_STATE_OCCLUDED))
        {
            pRT->BeginDraw();
            pRT->SetTransform(D2D1::Matrix3x2F::Identity());
            pRT->Clear(D2D1::ColorF(D2D1::ColorF::Black));
            pRT->EndDraw();
            if (hr == D2DERR_RECREATE_TARGET)
            {
                hr = S_OK;
                DiscardDeviceResources();
            }
        }
        InvalidateRect(hWnd, NULL, FALSE);
    }

    std::vector<float> RandomRadius(std::tuple<float, float> range, size_t count)
    {
        std::vector<float> radius_container{};

        dist.param(std::uniform_real_distribution<float>{ std::get<0>(range), std::get<1>(range) });
        rand_engine.seed(rand_device());
        dist(rand_engine);

        for (size_t i{}; i < count; i++)
        {
            radius_container.push_back(dist(rand_engine));
        }
    }

    std::vector<D2D1_ELLIPSE> CreateParticles(D2D1_POINT_2F& center, size_t particle_count)
    {
        auto radius_container = RandomRadius(std::make_tuple(3.0f, 8.0f), particle_count);
        std::vector<D2D1_ELLIPSE> ellipsese_container{};        

        for (size_t i{}; i < particle_count; i++)
        {            
            ellipsese_container.push_back(D2D1::Ellipse(center, radius_container[i],radius_container[i]));
        }
    }

    void DrawParticles(void)
    {
        GetClientRect(hWnd, &rc);

        const float radius = 5;
        const size_t particle_count = 10;
        const D2D1_POINT_2F center = D2D1::Point2(float((rc.left+rc.right)/2), float((rc.top+rc.bottom)/2));
        const D2D1_ELLIPSE ellipse = D2D1::Ellipse(center, radius, radius);
        std::vector<D2D1_ELLIPSE> ellipses_container(particle_count, ellipse);
        std::vector<D2D1::Matrix3x2F> trans_container{};

        float cx = 0;// (rc.left + rc.right) / 2;
        float cy = 0;// (rc.left + rc.right) / 2;
        static float bloom_radius = 10;
        if (bloom_radius >= float((rc.left + rc.right) / 2) - radius)
        {
            bloom_radius = 10;
        }
        bloom_radius++;

        for (size_t i{}; i < particle_count; i++)
        {
            float angle = 360 / float(particle_count) * i;
            float radians = angle * PI / 180;

            float vx = cx + cos(radians) * bloom_radius;
            float vy = cy + sin(radians) * bloom_radius;
            trans_container.push_back(D2D1::Matrix3x2F::Translation(vx, vy));
        }

        HRESULT hr;

        hr = CreateDeviceResources();
        if (SUCCEEDED(hr) && !(pRT->CheckWindowState() & D2D1_WINDOW_STATE_OCCLUDED))
        {
            pRT->BeginDraw();

            pRT->SetTransform(D2D1::Matrix3x2F::Identity());
            pRT->Clear(D2D1::ColorF(D2D1::ColorF::Black));

            for (size_t i{};i<particle_count;i++)
            {
                pRT->SetTransform(trans_container[i]);
                pRT->FillEllipse(ellipses_container[i], pFaceBrush);
                pRT->DrawEllipse(ellipses_container[i], pEdgeBrush);
            }
            pRT->EndDraw();

            if (hr == D2DERR_RECREATE_TARGET)
            {
                hr = S_OK;
                DiscardDeviceResources();
            }
        }
        InvalidateRect(hWnd, NULL, FALSE);
    }

    void DrawParticle(void)
    {
        GetClientRect(hWnd, &rc);

        const float radius = 5;
        const D2D1_POINT_2F center = D2D1::Point2(float((rc.left+rc.right)/2), float((rc.top+rc.bottom)/2));
        const D2D1_ELLIPSE ellipse = D2D1::Ellipse(center, radius, radius);

        static float x = 0;
        static float y = 0;
        if (x >= float((rc.left + rc.right) / 2) - 5.0f)
        {
            x = 0;
            y = 0;
        }
        x += 1;
        y += 1;

        HRESULT hr;

        hr = CreateDeviceResources();
        if (SUCCEEDED(hr) && !(pRT->CheckWindowState() & D2D1_WINDOW_STATE_OCCLUDED))
        {
            pRT->BeginDraw();

            pRT->SetTransform(D2D1::Matrix3x2F::Identity());
            pRT->Clear(D2D1::ColorF(D2D1::ColorF::Black));

            pRT->SetTransform(D2D1::Matrix3x2F::Translation(x, y));
            pRT->FillEllipse(ellipse, pFaceBrush);
            pRT->DrawEllipse(ellipse, pEdgeBrush);

            pRT->EndDraw();

            if (hr == D2DERR_RECREATE_TARGET)
            {
                hr = S_OK;
                DiscardDeviceResources();
            }
        }
        InvalidateRect(hWnd, NULL, FALSE);
    }

    RECT GetRect(void)
    {
        return rc;
    }
    //void DrawParticles(float x, float y, float radius)
    //{
    //    const D2D1_POINT_2F center = D2D1::Point2(x, y);
    //    const D2D1_ELLIPSE ellipse = D2D1::Ellipse(center, radius, radius);

    //    pRT->BeginDraw();

    //    pRT->FillEllipse(&ellipse, pEdgeBrush);

    //    pRT->EndDraw();
    //}
private:
	ID2D1Factory* pD2DFactory;
	HRESULT hr;
    HWND hWnd;
    ID2D1HwndRenderTarget* pRT;
    ID2D1SolidColorBrush* pEdgeBrush;
    ID2D1SolidColorBrush* pFaceBrush;
    RECT rc;

    std::uniform_real_distribution<float> dist;
    std::random_device rand_device;
    std::mt19937 rand_engine;
};

class FireWorks
{
public:
    FireWorks(std::shared_ptr<Drawer>& drawer) :drawer{ drawer }, particle_count{}, bloom_radius{}, bloom_radius_lm{}, particle_radius{}, radius_change{}
    {
        particle_count = 2;
        particle_radius = 5;
        bloom_radius = 20.0f;
        bloom_radius_lm = 250 - particle_radius;
        //drawer->CreateSolidBrush(D2D1::ColorF::White);
    }
    ~FireWorks()
    {
        
    }

    //void bloom(void)
    //{
    //    auto rc = drawer->GetRect();
    //    double cx = (rc.left + rc.right) / 2;
    //    double cy = (rc.left + rc.right) / 2;

    //    //drawer->Clear();
    //    for (size_t i{}; i < particle_count; i++)
    //    {
    //        auto angle = 360 / particle_count * i;
    //        auto radians = angle * PI / 180;

    //        auto vx = cx + cos(radians) * bloom_radius;
    //        auto vy = cy + sin(radians) * bloom_radius;

    //        drawer->DrawParticle(float(vx), float(vy));
    //    }        
    //}

    void change_radius(void)
    {
        if (bloom_radius >= bloom_radius_lm)
        {
            bloom_radius = 20.f;
        }
        else
        {
            bloom_radius+=10;
        }
    }

private:
    std::shared_ptr<Drawer> drawer;
    size_t particle_count;
    float bloom_radius;
    float bloom_radius_lm;
    float particle_radius;
    std::thread radius_change;
};