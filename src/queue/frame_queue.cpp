#include "frame_queue.h"

extern "C" {
#include <libavutil/frame.h>
#include <libavutil/mem.h>
}

FrameQueue::FrameQueue() = default;

FrameQueue::~FrameQueue()
{
    for (int i = 0; i < max_size_; i++) {
        Frame *vp = &queue_[i];
        unref_item(vp);
        av_frame_free(&vp->frame);
    }
}

int FrameQueue::init(int max_size, int keep_last)
{
    max_size_ = FFMIN(max_size, FRAME_QUEUE_SIZE);
    keep_last_ = !!keep_last;
    for (int i = 0; i < max_size_; i++) {
        /* Match C ffplay memset: AVSubtitle must be zero before avsubtitle_free. */
        queue_[i] = {};
        if (!(queue_[i].frame = av_frame_alloc()))
            return AVERROR(ENOMEM);
    }
    return 0;
}

void FrameQueue::abort()
{
    std::lock_guard lock(mutex_);
    abort_request_ = 1;
    cond_.notify_one();
}

void FrameQueue::start()
{
    std::lock_guard lock(mutex_);
    abort_request_ = 0;
}

Frame *FrameQueue::peek()
{
    return &queue_[(rindex_ + rindex_shown_) % max_size_];
}

Frame *FrameQueue::peek_next()
{
    return &queue_[(rindex_ + rindex_shown_ + 1) % max_size_];
}

Frame *FrameQueue::peek_last()
{
    return &queue_[rindex_];
}

Frame *FrameQueue::peek_writable()
{
    std::unique_lock lock(mutex_);
    while (size_ >= max_size_ && !abort_request_)
        cond_.wait(lock);

    if (abort_request_)
        return nullptr;

    return &queue_[windex_];
}

Frame *FrameQueue::peek_readable()
{
    std::unique_lock lock(mutex_);
    while (size_ - rindex_shown_ <= 0 && !abort_request_)
        cond_.wait(lock);

    if (abort_request_)
        return nullptr;

    return &queue_[(rindex_ + rindex_shown_) % max_size_];
}

void FrameQueue::push()
{
    if (++windex_ == max_size_)
        windex_ = 0;
    std::lock_guard lock(mutex_);
    size_++;
    cond_.notify_one();
}

void FrameQueue::next()
{
    if (keep_last_ && !rindex_shown_) {
        rindex_shown_ = 1;
        return;
    }
    unref_item(&queue_[rindex_]);
    if (++rindex_ == max_size_)
        rindex_ = 0;
    std::lock_guard lock(mutex_);
    size_--;
    cond_.notify_one();
}

int FrameQueue::nb_remaining() const
{
    return size_ - rindex_shown_;
}

int64_t FrameQueue::last_pos(int queue_serial) const
{
    const Frame *fp = &queue_[rindex_];
    if (rindex_shown_ && fp->serial == queue_serial)
        return fp->pos;
    return -1;
}

void FrameQueue::unref_item(Frame *vp)
{
    av_frame_unref(vp->frame);
    avsubtitle_free(&vp->sub);
}
