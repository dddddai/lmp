// SPDX-License-Identifier: GPL-2.0 OR BSD-3-Clause
/* Copyright (c) 2020 Facebook */
#include "vmlinux.h"
#include <bpf/bpf_helpers.h>
#include <bpf/bpf_tracing.h>
#include <bpf/bpf_core_read.h>
#include "paf.h"

char LICENSE[] SEC("license") = "Dual BSD/GPL";

struct {
	__uint(type, BPF_MAP_TYPE_RINGBUF);
	__uint(max_entries, 1);
} rb SEC(".maps");

SEC("kprobe/get_page_from_freelist")
int BPF_KPROBE(get_page_from_freelist, gfp_t gfp_mask, unsigned int order, int alloc_flags, const struct alloc_context *ac)
{
	struct event *e; 
	unsigned long *t, y;
	int a;

	e = bpf_ringbuf_reserve(&rb, sizeof(*e), 0);
	if (!e)
		return 0;
	y = BPF_CORE_READ(ac, preferred_zoneref, zone, watermark_boost);
	t = BPF_CORE_READ(ac, preferred_zoneref, zone, _watermark);

	e->present = BPF_CORE_READ(ac, preferred_zoneref, zone, present_pages);
	e->min = t[0] + y;
	e->low = t[1] + y;
	e->high = t[2] + y;
	e->flag = (int)gfp_mask;

	bpf_ringbuf_submit(e, 0);
	return 0;
}

