#ifndef UVC_DEVICE_H
#define UVC_DEVICE_H

#include <linux/version.h>
#include <media/v4l2-common.h>
#include <media/v4l2-device.h>
#include <media/v4l2-ioctl.h>
#include <media/v4l2-rect.h>
#include <media/videobuf2-core.h>
#include <media/videobuf2-v4l2.h>

#include "uvc.h"

#define PIXFMTS_MAX 4
#define FB_NAME_MAXLENGTH 16

#if LINUX_VERSION_CODE < KERNEL_VERSION(5, 7, 0)
#define VFL_TYPE_VIDEO VFL_TYPE_GRABBER
#define HD_720_WIDTH 1280
#define HD_720_HEIGHT 720
#endif

struct uvc_in_buffer {
    void *data;
    size_t filled;
    size_t xbar, ybar;
    uint32_t jiffies;
};

struct uvc_in_queue {
    struct uvc_in_buffer buffers[2];
    struct uvc_in_buffer dummy;
    struct uvc_in_buffer *pending;
    struct uvc_in_buffer *ready;
};

struct uvc_out_buffer {
    struct vb2_v4l2_buffer vb;
    struct list_head list;
    size_t filled;
};

struct uvc_out_queue {
    struct list_head active;
    int frame;
    /* TODO: implement more */
};

struct uvc_device_format {
    char *name;
    int fourcc;
    int bit_depth;
};

struct uvc_device {
    dev_t dev_number;
    struct v4l2_device v4l2_dev;
    struct video_device vdev;

    /* input buffer */
    struct uvc_in_queue in_queue;
    spinlock_t in_q_slock;
    spinlock_t in_fh_slock;
    bool fb_isopen;

    /* output buffer */
    struct vb2_queue vb_out_vidq;
    struct uvc_out_queue uvc_out_vidq;
    spinlock_t out_q_slock;
    /* Output framerate */
    struct v4l2_fract output_fps;

    char frames_dir[256];
    int frame_count;

    /* Input framebuffer */
    char uvc_fb_fname[FB_NAME_MAXLENGTH];
    struct proc_dir_entry *uvc_fb_procf;
    struct mutex uvc_mutex;

    /* framebuffer private data */
    void *fb_priv;

    /* Submitter thread */
    struct task_struct *sub_thr_id;

    /* Format descriptor */
    size_t nr_fmts;
    struct uvc_device_format out_fmts[PIXFMTS_MAX];

    struct uvc_device_spec fb_spec;
    struct v4l2_pix_format output_format;
    struct v4l2_pix_format input_format;

    /* Conversion switches */
    bool conv_pixfmt_on;
    bool conv_res_on;
    bool conv_crop_on;
};

struct uvc_device *create_uvc_device(size_t idx, struct uvc_device_spec *dev_spec);

int modify_uvc_device(struct uvc_device *uvc, struct uvc_device_spec *dev_spec);

void destroy_uvc_device(struct uvc_device *uvc);

int submitter_thread(void *data);

void fill_v4l2pixfmt(struct v4l2_pix_format *fmt, struct uvc_device_spec *dev_spec);

#endif
