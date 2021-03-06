/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
** Contact: http://www.qt.io/licensing/
**
** This file is part of the QtVulkan module
**
** $QT_BEGIN_LICENSE:LGPL3$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see http://www.qt.io/terms-conditions. For further
** information use the contact form at http://www.qt.io/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 3 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPLv3 included in the
** packaging of this file. Please review the following information to
** ensure the GNU Lesser General Public License version 3 requirements
** will be met: https://www.gnu.org/licenses/lgpl.html.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 2.0 or later as published by the Free
** Software Foundation and appearing in the file LICENSE.GPL included in
** the packaging of this file. Please review the following information to
** ensure the GNU General Public License version 2.0 requirements will be
** met: http://www.gnu.org/licenses/gpl-2.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#ifndef QVULKANRENDERLOOP_P_H
#define QVULKANRENDERLOOP_P_H

#include "qvulkanrenderloop.h"
#include <QObject>
#include <QQueue>
#include <QThread>
#include <QMutex>
#include <QWaitCondition>

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists for the convenience
// of a number of Qt sources files.  This header file may change from
// version to version without notice, or even be removed.
//
// We mean it.
//

QT_BEGIN_NAMESPACE

class QVulkanRenderThread;

class QVulkanRenderLoopPrivate : public QObject
{
public:
    QVulkanRenderLoopPrivate(QVulkanRenderLoop *q_ptr, QWindow *window);
    ~QVulkanRenderLoopPrivate();

    bool eventFilter(QObject *watched, QEvent *event) override;

    void postThreadEvent(QEvent *e, bool lock = true);

    void init();
    void cleanup();
    void recreateSwapChain();
    void ensureFrameCmdBuf(int frame, int subIndex);
    void submitFrameCmdBuf(VkSemaphore waitSem, VkSemaphore signalSem, int subIndex, bool fence);
    bool beginFrame();
    void endFrame();
    void renderFrame();

    void createDeviceAndSurface();
    void releaseDeviceAndSurface();
    void createSurface();
    void releaseSurface();
    bool physicalDeviceSupportsPresent(int queueFamilyIdx);

    void transitionImage(VkCommandBuffer cmdBuf, VkImage image,
                         VkImageLayout oldLayout, VkImageLayout newLayout,
                         VkAccessFlags srcAccessMask, VkAccessFlags dstAccessMask, bool ds = false);

    QVulkanRenderLoop *q;
    QVulkanRenderLoop::Flags m_flags = 0;
    int m_framesInFlight = 1;
    QVulkanRenderThread *m_thread = nullptr;
    QVulkanFrameWorker *m_worker = nullptr;
    QVulkanFunctions *f;

    WId m_winId;
#ifdef Q_OS_LINUX
    xcb_connection_t *m_xcbConnection;
    xcb_visualid_t m_xcbVisualId;
#endif
    QSize m_windowSize;
    bool m_inited = false;

    PFN_vkCreateDebugReportCallbackEXT vkCreateDebugReportCallbackEXT;
    PFN_vkDestroyDebugReportCallbackEXT vkDestroyDebugReportCallbackEXT;
    PFN_vkDebugReportMessageEXT vkDebugReportMessageEXT;

    VkFormat m_colorFormat;
    VkFormat m_dsFormat;
    VkInstance m_vkInst;
    VkPhysicalDevice m_vkPhysDev;
    VkPhysicalDeviceProperties m_physDevProps;
    VkPhysicalDeviceMemoryProperties m_vkPhysDevMemProps;
    VkDevice m_vkDev;
    VkQueue m_vkQueue;
    VkCommandPool m_vkCmdPool;
    uint32_t m_hostVisibleMemIndex;
    bool m_hasDebug;
    VkDebugReportCallbackEXT m_debugCallback;

    VkSurfaceKHR m_surface;
    VkSwapchainKHR m_swapChain = VK_NULL_HANDLE;
    uint32_t m_swapChainBufferCount = 0;

    static const int MAX_SWAPCHAIN_BUFFERS = 3;
    static const int MAX_FRAMES_IN_FLIGHT = 3;

    VkImage m_swapChainImages[MAX_SWAPCHAIN_BUFFERS];
    VkImageView m_swapChainImageViews[MAX_SWAPCHAIN_BUFFERS];
    VkDeviceMemory m_dsMem = VK_NULL_HANDLE;
    VkImage m_ds;
    VkImageView m_dsView;

    bool m_frameActive;
    VkSemaphore m_acquireSem[MAX_FRAMES_IN_FLIGHT];
    VkSemaphore m_renderSem[MAX_FRAMES_IN_FLIGHT];
    VkSemaphore m_workerWaitSem[MAX_FRAMES_IN_FLIGHT];
    VkSemaphore m_workerSignalSem[MAX_FRAMES_IN_FLIGHT];
    VkCommandBuffer m_frameCmdBuf[MAX_FRAMES_IN_FLIGHT][2];
    bool m_frameCmdBufRecording[MAX_FRAMES_IN_FLIGHT];
    VkFence m_frameFence[MAX_FRAMES_IN_FLIGHT];
    bool m_frameFenceActive[MAX_FRAMES_IN_FLIGHT];

    uint32_t m_currentSwapChainBuffer;
    uint32_t m_currentFrame;

#if defined(Q_OS_WIN)
    PFN_vkCreateWin32SurfaceKHR vkCreateWin32SurfaceKHR;
    PFN_vkGetPhysicalDeviceWin32PresentationSupportKHR vkGetPhysicalDeviceWin32PresentationSupportKHR;
#elif defined(Q_OS_LINUX)
    PFN_vkCreateXcbSurfaceKHR vkCreateXcbSurfaceKHR;
    PFN_vkGetPhysicalDeviceXcbPresentationSupportKHR vkGetPhysicalDeviceXcbPresentationSupportKHR;
#endif

    PFN_vkDestroySurfaceKHR vkDestroySurfaceKHR;
    PFN_vkGetPhysicalDeviceSurfaceSupportKHR vkGetPhysicalDeviceSurfaceSupportKHR;
    PFN_vkGetPhysicalDeviceSurfaceCapabilitiesKHR vkGetPhysicalDeviceSurfaceCapabilitiesKHR;
    PFN_vkGetPhysicalDeviceSurfaceFormatsKHR vkGetPhysicalDeviceSurfaceFormatsKHR;
    PFN_vkGetPhysicalDeviceSurfacePresentModesKHR vkGetPhysicalDeviceSurfacePresentModesKHR;

    PFN_vkCreateSwapchainKHR vkCreateSwapchainKHR = nullptr;
    PFN_vkDestroySwapchainKHR vkDestroySwapchainKHR;
    PFN_vkGetSwapchainImagesKHR vkGetSwapchainImagesKHR;
    PFN_vkAcquireNextImageKHR vkAcquireNextImageKHR;
    PFN_vkQueuePresentKHR vkQueuePresentKHR;
};

class QVulkanRenderThreadEventQueue : public QQueue<QEvent *>
{
public:
    void addEvent(QEvent *e);
    QEvent *takeEvent(bool wait);
    bool hasMoreEvents();

private:
    QMutex m_mutex;
    QWaitCondition m_condition;
    bool m_waiting = false;
};

class QVulkanRenderThreadExposeEvent : public QEvent
{
public:
    static const QEvent::Type Type = QEvent::Type(QEvent::User + 1);
    QVulkanRenderThreadExposeEvent() : QEvent(Type) { }
};

class QVulkanRenderThreadObscureEvent : public QEvent
{
public:
    static const QEvent::Type Type = QEvent::Type(QEvent::User + 2);
    QVulkanRenderThreadObscureEvent() : QEvent(Type) { }
};

class QVulkanRenderThreadResizeEvent : public QEvent
{
public:
    static const QEvent::Type Type = QEvent::Type(QEvent::User + 3);
    QVulkanRenderThreadResizeEvent() : QEvent(Type) { }
};

class QVulkanRenderThreadUpdateEvent : public QEvent
{
public:
    static const QEvent::Type Type = QEvent::Type(QEvent::User + 4);
    QVulkanRenderThreadUpdateEvent() : QEvent(Type) { }
};

class QVulkanRenderThreadFrameQueuedEvent : public QEvent
{
public:
    static const QEvent::Type Type = QEvent::Type(QEvent::User + 5);
    QVulkanRenderThreadFrameQueuedEvent() : QEvent(Type) { }
};

class QVulkanRenderThreadDestroyEvent : public QEvent
{
public:
    static const QEvent::Type Type = QEvent::Type(QEvent::User + 6);
    QVulkanRenderThreadDestroyEvent() : QEvent(Type) { }
};

class QVulkanRenderThread : public QThread
{
public:
    QVulkanRenderThread(QVulkanRenderLoopPrivate *d_ptr) : d(d_ptr) { }

    void run() override;

    void processEvents();
    void processEventsAndWaitForMore();
    void postEvent(QEvent *e);

    QMutex *mutex() { return &m_mutex; }
    QWaitCondition *waitCondition() { return &m_condition; }
    void setActive() { m_active = true; }
    void setUpdatePending();

private:
    void processEvent(QEvent *e);
    void obscure();
    void resize();

    QVulkanRenderLoopPrivate *d;
    QVulkanRenderThreadEventQueue m_eventQueue;
    volatile bool m_active;
    QMutex m_mutex;
    QWaitCondition m_condition;
    uint m_sleeping : 1;
    uint m_stopEventProcessing : 1;
    uint m_pendingUpdate : 1;
    uint m_pendingObscure : 1;
    uint m_pendingResize : 1;
    uint m_pendingDestroy : 1;
};

QT_END_NAMESPACE

#endif // QVULKANRENDERLOOP_P_H
