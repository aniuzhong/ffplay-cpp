#pragma once

#include <condition_variable>
#include <mutex>

#include "frame.h"

constexpr int FRAME_QUEUE_SIZE = 16;  // max(SUBPICTURE=16, SAMPLE=9, VIDEO=3)

class FrameQueue {
public:
    FrameQueue();
    ~FrameQueue();

    FrameQueue(const FrameQueue&) = delete;
    FrameQueue& operator=(const FrameQueue&) = delete;

    int init(int max_size, int keep_last);  // returns 0 or AVERROR(ENOMEM)

    // Own abort/start pair — replaces reading pktq->abort_request.
    // Caller (decoder_abort / decoder_start) must keep these in sync
    // with the corresponding PacketQueue so the decode pipeline
    // unblocks and restarts as one unit.
    void abort();  // sets abort flag, notifies blocked peek_*
    void start();  // clears abort flag (paired with abort across reopen)

    Frame* peek();
    Frame* peek_next();
    Frame* peek_last();
    Frame* peek_writable();   // blocks; returns nullptr if aborted
    Frame* peek_readable();   // blocks; returns nullptr if aborted
    void push();
    void next();

    int nb_remaining() const;
    int rindex_shown() const { return rindex_shown_; }
    std::mutex& mutex_ref() { return mutex_; }  // for external lock around peek()+update_video_pts

    // queue_serial: the current serial of the associated PacketQueue.
    // Was read directly from f->pktq->serial(); now passed explicitly.
    int64_t last_pos(int queue_serial) const;

private:
    static void unref_item(Frame *vp);

    Frame queue_[FRAME_QUEUE_SIZE];
    int rindex_ = 0;
    int windex_ = 0;
    int size_ = 0;
    int max_size_ = 0;
    int keep_last_ = 0;
    int rindex_shown_ = 0;
    int abort_request_ = 0;
    std::mutex mutex_;
    std::condition_variable cond_;
};
