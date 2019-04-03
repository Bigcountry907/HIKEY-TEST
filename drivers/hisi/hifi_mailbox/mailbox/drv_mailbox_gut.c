#include "drv_mailbox_cfg.h"
#include "drv_mailbox_platform.h"
#include "drv_mailbox_debug.h"
#include "drv_mailbox_gut.h"
#include <asm/io.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/kernel.h>
#include <linux/platform_device.h>
#include <linux/fb.h>
#include <linux/console.h>
#include <linux/uaccess.h>
#include <linux/kthread.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/types.h>
#include <linux/string.h>
#include <asm/pgtable.h>
#include<linux/vmalloc.h>
#include <asm/io.h>
#include <linux/mm.h>

extern int logMsg(char *fmt, ...);
#undef	_MAILBOX_FILE_
#define _MAILBOX_FILE_	 "gut"

#define SLICE_REG 0xFFF0A534
#define mem_remap_nocache(phys_addr, size) mem_remap_type(phys_addr, size, pgprot_writecombine(PAGE_KERNEL))
MAILBOX_EXTERN struct mb g_mailbox_handle = { MAILBOX_MAILCODE_INVALID };

void __iomem *g_shareMemBase;
void *g_slice_reg;

MAILBOX_LOCAL int mailbox_init_mem(struct mb_cfg *config);

MAILBOX_EXTERN int mailbox_queue_write(struct mb_queue *queue,
				       char *data, unsigned int size)
{
	unsigned int SizeToBottom;

	SizeToBottom =
	    (unsigned int)((queue->base + queue->length) - queue->front);

	if (SizeToBottom > size) {
		mailbox_memcpy((void *)queue->front, (const void *)data, size);

		queue->front += size;
	} else {
		mailbox_memcpy((void *)(queue->front), (const void *)data,
			       SizeToBottom);

		mailbox_memcpy((void *)(queue->base),
			       (const void *)(data + SizeToBottom),
			       (size - SizeToBottom));

		queue->front = (queue->base + size) - SizeToBottom;
	}

	return (int)size;
}

MAILBOX_GLOBAL int mailbox_queue_read(struct mb_queue *queue,
				      char *buff, unsigned int size)
{
	unsigned int to_bottom;

	to_bottom = (unsigned int)((queue->base + queue->length) - queue->rear);

	if (to_bottom > size) {
		mailbox_memcpy((void *)buff, (const void *)(queue->rear), size);

		queue->rear += size;
	} else {
		mailbox_memcpy((void *)buff, (const void *)(queue->rear),
			       to_bottom);

		mailbox_memcpy((void *)(buff + to_bottom),
			       (const void *)(queue->base), (size - to_bottom));

		queue->rear = (queue->base + size) - to_bottom;
	}

	return (int)size;
}

MAILBOX_LOCAL int mailbox_check_mail(struct mb_buff *mbuff,
				     struct mb_mail *msg_head,
				     unsigned long data_addr)
{
	unsigned int time_stamp;
	unsigned int seq_num;
	int ret_val = MAILBOX_OK;
	if (MAILBOX_MSGHEAD_NUMBER != msg_head->ulPartition) {
		ret_val =
		    mailbox_logerro_p1(MAILBOX_CRIT_GUT_MSG_CHECK_FAIL,
				       msg_head->ulMailCode);
	}

	seq_num = mbuff->seq_num;

	if (MAILBOX_SEQNUM_START == msg_head->ulSeqNum) {
		ret_val =
		    mailbox_logerro_p1(MAILBOX_INFO_RECEIVE_FIRST_MAIL,
				       msg_head->ulMailCode);
	} else if ((seq_num + 1) != msg_head->ulSeqNum) {
		ret_val =
		    mailbox_logerro_p1
		    (MAILBOX_ERR_GUT_MAILBOX_SEQNUM_CHECK_FAIL,
		     msg_head->ulMailCode);
	}

	mbuff->seq_num = msg_head->ulSeqNum;

	time_stamp = (unsigned int)mailbox_get_timestamp();

	msg_head->ulReadSlice = time_stamp;

	mailbox_record_transport(&(mbuff->mntn), msg_head->ulMailCode,
				 msg_head->ulWriteSlice, msg_head->ulReadSlice,
				 data_addr);
	return ret_val;
}

MAILBOX_EXTERN struct mb *mailbox_get_mb(void)
{
	if (MAILBOX_INIT_MAGIC == g_mailbox_handle.init_flag) {
		return &g_mailbox_handle;
	}

	mailbox_out(("error: mb not init\n"));
	return MAILBOX_NULL;
}

MAILBOX_EXTERN struct mb_buff *mailbox_get_channel_handle(struct mb *mailbox,
							  unsigned int mailcode)
{
	struct mb_link *link_array = MAILBOX_NULL;
	struct mb_buff *mbuff = MAILBOX_NULL;
	unsigned int src_id = 0;
	unsigned int dst_id = 0;
	unsigned int carrier_id = 0;

	src_id = mailbox_get_src_id(mailcode);
	dst_id = mailbox_get_dst_id(mailcode);
	carrier_id = mailbox_get_carrier_id(mailcode);

	if (src_id == mailbox->local_id) {
		if (dst_id < MAILBOX_CPUID_BUTT) {
			link_array = &mailbox->send_tbl[dst_id];
		} else {
			(void)
			    mailbox_logerro_p1
			    (MAILBOX_ERR_GUT_INVALID_CHANNEL_ID, mailcode);
			return MAILBOX_NULL;
		}
	} else if (dst_id == mailbox->local_id) {
		if (src_id < MAILBOX_CPUID_BUTT) {
			link_array = &mailbox->recv_tbl[src_id];
		} else {
			(void)
			    mailbox_logerro_p1
			    (MAILBOX_ERR_GUT_INVALID_CHANNEL_ID, mailcode);
			return MAILBOX_NULL;
		}
	}

	if ((MAILBOX_NULL == link_array) || (0 == link_array->carrier_butt)) {
		(void)mailbox_logerro_p1(MAILBOX_ERR_GUT_INVALID_CPU_LINK,
					 mailcode);
		return MAILBOX_NULL;
	}

	if (carrier_id < link_array->carrier_butt) {
		mbuff = &link_array->channel_buff[carrier_id];
	} else {
		(void)mailbox_logerro_p1(MAILBOX_ERR_GUT_INVALID_CARRIER_ID,
					 mailcode);
		return MAILBOX_NULL;
	}

	if (mailbox_get_use_id(mailcode) >=
	    mailbox_get_use_id(mbuff->config->butt_id)) {
		(void)mailbox_logerro_p1(MAILBOX_ERR_GUT_INVALID_USER_ID,
					 mailcode);

		return MAILBOX_NULL;
	}

	return mbuff;
}

MAILBOX_LOCAL int mailbox_request_channel(struct mb *mb,
					  struct mb_buff **mb_buf,
					  unsigned int mailcode)
{
	struct mb_head *head = MAILBOX_NULL;
	struct mb_queue *queue = MAILBOX_NULL;
	struct mb_buff *mbuff = MAILBOX_NULL;
	int ret_val = MAILBOX_OK;

	mailbox_dpm_device_get();

	*mb_buf = MAILBOX_NULL;
	mbuff = mailbox_get_channel_handle(mb, mailcode);
	if (MAILBOX_NULL == mbuff) {
		ret_val = (int)MAILBOX_ERRO;
		goto request_erro;
	}

	head = (struct mb_head *)mbuff->config->head_addr;
	if (((MAILBOX_PROTECT1 != head->ulProtectWord1))
	    || (MAILBOX_PROTECT2 != head->ulProtectWord2)
	    || (MAILBOX_PROTECT1 != head->ulProtectWord3)
	    || (MAILBOX_PROTECT2 != head->ulProtectWord4)) {
		/* do not check cause hifi maybe not pwr on when the first ipc message send */

		mailbox_init_mem(mbuff->config);

		mailbox_logerro_p1(MAILBOX_NOT_READY, mailcode);
	}

	if (mailbox_get_src_id(mailcode) == mb->local_id) {
		if (MAILBOX_TRUE == mailbox_int_context()) {
			ret_val =
			    mailbox_logerro_p1
			    (MAILBOX_ERR_GUT_SEND_MAIL_IN_INT_CONTEXT,
			     mailcode);
			goto request_erro;
		} else {
			if (MAILBOX_OK != mailbox_mutex_lock(&mbuff->mutex)) {
				ret_val =
				    mailbox_logerro_p1
				    (MAILBOX_CRIT_GUT_MUTEX_LOCK_FAILED,
				     mailcode);
				goto request_erro;
			}
		}
	}

	mbuff->mailcode = mailcode;

	queue = &mbuff->mail_queue;
	queue->front = queue->base + head->ulFront * sizeof(unsigned int);
	queue->rear = queue->base + head->ulRear * sizeof(unsigned int);

	mbuff->mb = mb;
	*mb_buf = mbuff;
	return MAILBOX_OK;
 request_erro:
	mailbox_out(("###mailbox_request_channel ERR! \n"));
	mailbox_dpm_device_put();
	return ret_val;
}

MAILBOX_LOCAL int mailbox_release_channel(struct mb *mb, struct mb_buff *mbuff)
{
	unsigned int channel_id = mbuff->channel_id;

	if (mb->local_id == mailbox_get_src_id(channel_id)) {
		if (MAILBOX_TRUE == mailbox_int_context()) {
		} else {
			mailbox_mutex_unlock(&mbuff->mutex);
		}
	}

	mailbox_dpm_device_put();
	return MAILBOX_OK;
}

MAILBOX_LOCAL int mailbox_read_mail(struct mb_buff *mbuff)
{
	struct mb_mail mail;
	struct mb_cb *read_cb = MAILBOX_NULL;
	struct mb_queue tmp_queue;
	struct mb_queue *usr_queue = MAILBOX_NULL;
	struct mb_queue *m_queue = MAILBOX_NULL;
	unsigned int use_id;
	unsigned int slice_start;
	unsigned int to_bottom;
	void *usr_handle;
	void *usr_data;
	void (*usr_func) (void *mbuf, void *handle, void *data);

	m_queue = &(mbuff->mail_queue);
	usr_queue = &(mbuff->usr_queue);
	mailbox_memcpy((void *)usr_queue, (const void *)m_queue,
		       sizeof(struct mb_queue));

	tmp_queue.base = usr_queue->base;
	tmp_queue.length = usr_queue->length;
	tmp_queue.front = usr_queue->rear;
	tmp_queue.rear = usr_queue->front;

	mailbox_queue_read(usr_queue, (char *)&mail, sizeof(struct mb_mail));

	mailbox_check_mail(mbuff, &mail, m_queue->rear);

	mailbox_queue_write(&tmp_queue, (char *)&mail, sizeof(struct mb_mail));
	use_id = mailbox_get_use_id(mail.ulMailCode);

	if (use_id >= mailbox_get_use_id(mbuff->config->butt_id)) {
		(void)mailbox_logerro_p1(MAILBOX_ERR_GUT_INVALID_USER_ID,
					 mail.ulMailCode);
		goto EXIT;
	}

	read_cb = &mbuff->read_cb[use_id];
	mailbox_mutex_lock(&mbuff->mutex);
	usr_handle = read_cb->handle;
	usr_data = read_cb->data;
	usr_func = read_cb->func;
	mailbox_mutex_unlock(&mbuff->mutex);
	if (MAILBOX_NULL != usr_func) {
		usr_queue->size = mail.ulMsgLength;
		slice_start = (unsigned int)mailbox_get_timestamp();
		usr_func(mbuff, usr_handle, usr_data);

		mailbox_record_receive(&mbuff->mntn, mail.ulMailCode,
				       slice_start);
	} else {
		(void)mailbox_logerro_p1(MAILBOX_ERR_GUT_READ_CALLBACK_NOT_FIND,
					 mail.ulMailCode);
	}

 EXIT:

	to_bottom =
	    (unsigned int)((m_queue->base + m_queue->length) - m_queue->rear);
	if (to_bottom > (mail.ulMsgLength + sizeof(struct mb_mail))) {
		m_queue->rear += (mail.ulMsgLength + sizeof(struct mb_mail));
	} else {
		m_queue->rear = m_queue->base + ((mail.ulMsgLength +
						  sizeof(struct mb_mail)) -
						 to_bottom);
	}

	m_queue->rear = mailbox_align_size(m_queue->rear, MAILBOX_ALIGN);

	return (int)(mailbox_align_size(mail.ulMsgLength, MAILBOX_ALIGN)
		     + sizeof(struct mb_mail));

}

MAILBOX_LOCAL int mailbox_receive_all_mails(struct mb_buff *mbuf)
{

	struct mb_queue *queue;
	signed int size_left;
	unsigned int mail_len = 0;
	int ret_val = MAILBOX_OK;

	queue = &(mbuf->mail_queue);

	if (queue->front >= queue->rear) {
		size_left = (signed int)(queue->front - queue->rear);
	} else {
		size_left = (signed int)((queue->length) - ((queue->rear)
							    - (queue->front)));
	}

	while (size_left > 0) {
		mail_len = (unsigned int)mailbox_read_mail(mbuf);
		if (0 == mail_len) {
			ret_val =
			    mailbox_logerro_p1(MAILBOX_CRIT_GUT_READ_MAIL,
					       mbuf->channel_id);
		}
		size_left -= (signed int)(mail_len);
	}

	if (size_left < 0) {
		return mailbox_logerro_p1(MAILBOX_CRIT_GUT_RECEIVE_MAIL,
					  mbuf->channel_id);
	}
	return ret_val;
}

MAILBOX_LOCAL int mailbox_read_channel(unsigned int channel_id)
{
	struct mb_buff *mbuf = MAILBOX_NULL;
	struct mb *mb = MAILBOX_NULL;
	int ret_val = MAILBOX_OK;

	mb = mailbox_get_mb();

	if (MAILBOX_NULL == mb) {
		return (int)MAILBOX_ERRO;
	}

	if (mb->local_id != mailbox_get_dst_id(channel_id)) {
		return mailbox_logerro_p1(MAILBOX_ERR_GUT_INVALID_TARGET_CPU,
					  channel_id);
	}

	ret_val = mailbox_request_channel(mb, &mbuf, channel_id);
	if (MAILBOX_OK != ret_val) {
		return ret_val;
	}

	ret_val = mailbox_receive_all_mails(mbuf);

	mailbox_flush_buff(mbuf);

	if (MAILBOX_OK != mailbox_release_channel(mb, mbuf)) {
		return mailbox_logerro_p1(MAILBOX_ERR_GUT_RELEASE_CHANNEL,
					  channel_id);
	}

	return ret_val;
}

MAILBOX_LOCAL int mailbox_init_mem(struct mb_cfg *config)
{
	struct mb_head *head = (struct mb_head *)config->head_addr;
	unsigned long data_addr = config->data_addr;
	unsigned int data_size = config->data_size;

	if ((0 == data_addr) || (MAILBOX_NULL == head) || (0 == data_size)) {
		return mailbox_logerro_p1(MAILBOX_ERR_GUT_INIT_CORESHARE_MEM,
					  config->butt_id);
	}

	mailbox_memset((signed char *)data_addr, 0, data_size);

	mailbox_write_reg(data_addr, MAILBOX_PROTECT1);
	mailbox_write_reg(data_addr + MAILBOX_PROTECT_LEN, MAILBOX_PROTECT2);

	mailbox_write_reg((data_addr + data_size) - (2 * MAILBOX_PROTECT_LEN),
			  MAILBOX_PROTECT1);
	mailbox_write_reg((data_addr + data_size) - MAILBOX_PROTECT_LEN,
			  MAILBOX_PROTECT2);

	head->ulFront = 0;
	head->ulFrontslice = 0;
	head->ulRear = head->ulFront;
	head->ulRearslice = 0;
	head->ulProtectWord4 = MAILBOX_PROTECT2;
	head->ulProtectWord3 = MAILBOX_PROTECT1;
	head->ulProtectWord2 = MAILBOX_PROTECT2;
	head->ulProtectWord1 = MAILBOX_PROTECT1;

	return MAILBOX_OK;
}

MAILBOX_LOCAL int mailbox_calculate_space(struct mb *mb,
					  struct mb_cfg *config,
					  unsigned int cpu_id)
{
	struct mb_link *send_link = MAILBOX_NULL;
	struct mb_link *recev_link = MAILBOX_NULL;
	int ret_val = MAILBOX_OK;
	unsigned int src_id = 0;
	unsigned int dst_id = 0;
	unsigned int carrier_id = 0;

	send_link = mb->send_tbl;
	recev_link = mb->recv_tbl;

	while (MAILBOX_MAILCODE_INVALID != config->butt_id) {
		src_id = mailbox_get_src_id(config->butt_id);
		dst_id = mailbox_get_dst_id(config->butt_id);
		carrier_id = mailbox_get_carrier_id(config->butt_id);

		if (cpu_id == src_id) {
			if (dst_id < MAILBOX_CPUID_BUTT) {
				if ((carrier_id + 1) >
				    send_link[dst_id].carrier_butt) {
					send_link[dst_id].carrier_butt =
					    (carrier_id + 1);
				}
			} else {
				ret_val =
				    mailbox_logerro_p1
				    (MAILBOX_ERR_GUT_INVALID_CHANNEL_ID,
				     config->butt_id);
			}
		}
		if (cpu_id == dst_id) {
			if (src_id < MAILBOX_CPUID_BUTT) {
				if ((carrier_id + 1) >
				    recev_link[src_id].carrier_butt) {
					recev_link[src_id].carrier_butt =
					    (carrier_id + 1);
				}
			} else {
				ret_val =
				    mailbox_logerro_p1
				    (MAILBOX_ERR_GUT_INVALID_CHANNEL_ID,
				     config->butt_id);
			}
		}

		config++;
	}

	return ret_val;
}

MAILBOX_LOCAL int mailbox_init_all_handle(struct mb *mb,
					  struct mb_cfg *config,
					  unsigned int cpu_id)
{
	struct mb_queue *queue = MAILBOX_NULL;
	unsigned int direct = MIALBOX_DIRECTION_INVALID;
	int ret_val = MAILBOX_OK;
	struct mb_link *send_link = MAILBOX_NULL;
	struct mb_link *recv_link = MAILBOX_NULL;
	struct mb_buff *mbuf_prob = &g_mailbox_channel_handle_pool[0];
	struct mb_cb *cb_prob = &g_mailbox_user_cb_pool[0];
	struct mb_buff *mbuf_cur = MAILBOX_NULL;
	unsigned int channel_sum = 0;
	unsigned int use_sum = 0;
	unsigned int src_id = 0;
	unsigned int dst_id = 0;
	unsigned int carrier_id = 0;
	unsigned int use_max = 0;

	send_link = mb->send_tbl;
	recv_link = mb->recv_tbl;

	while (MAILBOX_MAILCODE_INVALID != config->butt_id) {
		direct = MIALBOX_DIRECTION_INVALID;
		src_id = mailbox_get_src_id(config->butt_id);
		dst_id = mailbox_get_dst_id(config->butt_id);
		carrier_id = mailbox_get_carrier_id(config->butt_id);
		use_max = mailbox_get_use_id(config->butt_id);

		printk
		    ("******************func = %s line = %d src_id = %d dst_id = %d******************\n",
		     __func__, __LINE__, src_id, dst_id);

		if (cpu_id == src_id) {
			direct = MIALBOX_DIRECTION_SEND;

			if (MAILBOX_NULL == send_link[dst_id].channel_buff) {
				send_link[dst_id].channel_buff = mbuf_prob;
				mbuf_prob += (send_link[dst_id].carrier_butt);
				channel_sum += (send_link[dst_id].carrier_butt);
				if (channel_sum > MAILBOX_CHANNEL_NUM) {
					return
					    mailbox_logerro_p1
					    (MAILBOX_CRIT_GUT_INIT_CHANNEL_POOL_TOO_SMALL,
					     channel_sum);
				}
			}
			mbuf_cur = &send_link[dst_id].channel_buff[carrier_id];
		}

		if (cpu_id == dst_id) {
			direct = MIALBOX_DIRECTION_RECEIVE;

			if (MAILBOX_NULL == recv_link[src_id].channel_buff) {
				recv_link[src_id].channel_buff = mbuf_prob;
				mbuf_prob += (recv_link[src_id].carrier_butt);
				channel_sum += (recv_link[src_id].carrier_butt);
				if (channel_sum > MAILBOX_CHANNEL_NUM) {
					return
					    mailbox_logerro_p1
					    (MAILBOX_CRIT_GUT_INIT_CHANNEL_POOL_TOO_SMALL,
					     channel_sum);
				}
			}

			mbuf_cur = &recv_link[src_id].channel_buff[carrier_id];

			mbuf_cur->read_cb = cb_prob;
			cb_prob += use_max;
			use_sum += use_max;
			if (use_sum > MAILBOX_USER_NUM) {
				return
				    mailbox_logerro_p1
				    (MAILBOX_CRIT_GUT_INIT_USER_POOL_TOO_SMALL,
				     use_sum);
			}

			ret_val =
			    mailbox_process_register(mailbox_get_channel_id
						     (config->butt_id),
						     mailbox_read_channel,
						     mbuf_cur);

			ret_val |= mailbox_init_mem(config);

		}

		if ((MIALBOX_DIRECTION_INVALID != direct)
		    && (MAILBOX_NULL != mbuf_cur)) {
			mbuf_cur->channel_id =
			    mailbox_get_channel_id(config->butt_id);
			mbuf_cur->seq_num = MAILBOX_SEQNUM_START;
			mbuf_cur->mntn.peak_traffic_left =
			    MAILBOX_QUEUE_LEFT_INVALID;
			mbuf_cur->mntn.mbuff = mbuf_cur;
			mbuf_cur->config = config;

			queue = &(mbuf_cur->mail_queue);

			queue->length = mbuf_cur->config->data_size -
			    (MAILBOX_DATA_LEN_PROTECT_NUM *
			     MAILBOX_PROTECT_LEN);
			queue->base =
			    mbuf_cur->config->data_addr +
			    (MAILBOX_DATA_BASE_PROTECT_NUM *
			     MAILBOX_PROTECT_LEN);

			ret_val =
			    mailbox_channel_register(mailbox_get_channel_id
						     (config->butt_id),
						     config->int_src,
						     mailbox_get_dst_id(config->
									butt_id),
						     direct, &mbuf_cur->mutex);
		}

		config++;
	}

	/*g_mailbox_global_cfg_tbl[];
	   g_mailbox_channel_handle_pool[MAILBOX_CHANNEL_NUM];
	   g_mailbox_user_cb_pool[MAILBOX_USER_NUM];
	 */
	if ((unsigned int)MAILBOX_CHANNEL_NUM != channel_sum) {
		ret_val =
		    mailbox_logerro_p1
		    (MAILBOX_ERR_GUT_INIT_CHANNEL_POOL_TOO_LARGE,
		     ((MAILBOX_CHANNEL_NUM << 16) | channel_sum));
	}

	if (MAILBOX_USER_NUM != use_sum) {
		ret_val =
		    mailbox_logerro_p1(MAILBOX_ERR_GUT_INIT_USER_POOL_TOO_LARGE,
				       use_sum);
	}

	return ret_val;
}

MAILBOX_LOCAL int mailbox_create_box(struct mb *mb,
				     struct mb_cfg *config, unsigned int cpu_id)
{
	mb->local_id = cpu_id;

	printk("<<<<<<<<<<<<<<mailbox_create_box cpu_id = %d>>>>>>>>>>>>>>\n",
	       cpu_id);

	if (MAILBOX_OK != mailbox_calculate_space(mb, config, cpu_id)) {
		return mailbox_logerro_p0(MAILBOX_ERR_GUT_CALCULATE_SPACE);
	}

	return mailbox_init_all_handle(mb, config, cpu_id);
}

void *mem_remap_type(unsigned long phys_addr, size_t size, pgprot_t pgprot)
{
	int i;
	u8 *vaddr;
	int npages =
	    PAGE_ALIGN((phys_addr & (PAGE_SIZE - 1)) + size) >> PAGE_SHIFT;
	unsigned long offset = phys_addr & (PAGE_SIZE - 1);
	struct page **pages;
	pages = vmalloc(sizeof(struct page *) * npages);
	if (!pages) {
		printk(KERN_ERR "%s: vmalloc return NULL!\n", __FUNCTION__);
		return NULL;
	}
	pages[0] = phys_to_page(phys_addr);
	for (i = 0; i < npages - 1; i++) {
		pages[i + 1] = pages[i] + 1;
	}
	vaddr = (u8 *) vmap(pages, npages, VM_MAP, pgprot);
	if (vaddr == 0) {
		printk(KERN_ERR "%s: vmap return NULL!\n", __FUNCTION__);
	} else {
		vaddr += offset;
	}
	vfree(pages);
	printk(KERN_DEBUG
	       "%s: phys_addr:0x%08lx size:0x%08lx npages:%d vaddr:%pK offset:0x%08lx\n",
	       __FUNCTION__, phys_addr, (unsigned long)size, npages, vaddr,
	       offset);
	return (void *)vaddr;
}

MAILBOX_GLOBAL int mailbox_init(void)
{
	unsigned long offset = 0;
	int i = 0;
	struct mb_head *head = NULL;

	if (MAILBOX_INIT_MAGIC == g_mailbox_handle.init_flag) {
		return mailbox_logerro_p1(MAILBOX_ERR_GUT_ALREADY_INIT,
					  g_mailbox_handle.init_flag);
	}

	printk("func = %s, line = %d baseaddr = 0x%x\n", __func__, __LINE__,
	       (unsigned int)MAILBOX_MEM_BASEADDR);

	g_shareMemBase =
	    mem_remap_nocache(MAILBOX_MEM_BASEADDR, MAILBOX_MEM_LENGTH);

	if (NULL == g_shareMemBase) {
		printk("func = %s, mmap mailbox mem error!!\n", __func__);
		return mailbox_logerro_p0(MAILBOX_CRIT_GUT_MEMORY_CONFIG);
	}

	offset = (unsigned long)g_shareMemBase - MAILBOX_MEM_BASEADDR;
	for (i = 0; i < MAILBOX_GLOBAL_CHANNEL_NUM; i++) {
		g_mailbox_global_cfg_tbl[i].head_addr =
		    g_mailbox_global_cfg_tbl[i].head_addr + offset;
		g_mailbox_global_cfg_tbl[i].data_addr =
		    g_mailbox_global_cfg_tbl[i].data_addr + offset;

		printk("i = %d, head_addr = 0x%lx, data_addr = 0x%lx\n", i,
		       g_mailbox_global_cfg_tbl[i].head_addr,
		       g_mailbox_global_cfg_tbl[i].data_addr);

		head = (struct mb_head *)g_mailbox_global_cfg_tbl[i].head_addr;
		head->ulFront = 0x0;
		head->ulRear = 0x0;
		head->ulFrontslice = 0x0;
		head->ulRearslice = 0x0;
	}

	mailbox_memset(&g_mailbox_handle, 0x00, sizeof(struct mb));

	if ((MAILBOX_HEAD_BOTTOM_ADDR >
	     (MAILBOX_MEM_BASEADDR + MAILBOX_MEM_HEAD_LEN))
	    || (MAILBOX_MEMORY_BOTTOM_ADDR >
		(MAILBOX_MEM_BASEADDR + MAILBOX_MEM_LENGTH))) {
		mailbox_out(("mailbox address overflow: headbuttom valid(0x%lx), config(0x%x)!\n\
          \r                          databuttom valid(0x%x), config(0x%x)!" RT,
				(MAILBOX_MEM_BASEADDR + MAILBOX_MEM_HEAD_LEN), (unsigned int)MAILBOX_HEAD_BOTTOM_ADDR, (unsigned int)(MAILBOX_MEM_BASEADDR + MAILBOX_MEM_LENGTH),
				(unsigned int)MAILBOX_MEMORY_BOTTOM_ADDR));
		return mailbox_logerro_p0(MAILBOX_CRIT_GUT_MEMORY_CONFIG);
	}

	if (MAILBOX_OK != mailbox_create_box(&g_mailbox_handle,
					     &g_mailbox_global_cfg_tbl[0],
					     MAILBOX_LOCAL_CPUID)) {
		return mailbox_logerro_p0(MAILBOX_ERR_GUT_CREATE_BOX);
	}

	if (MAILBOX_OK != mailbox_init_platform()) {
		return mailbox_logerro_p0(MAILBOX_ERR_GUT_INIT_PLATFORM);
	}

	g_mailbox_handle.init_flag = MAILBOX_INIT_MAGIC;

/*mailbox_ifc_test_init();*/

/*fixme: there isn't go to unremap*/
	g_slice_reg = (void *)ioremap(SLICE_REG, 0x4);
	if (NULL == g_slice_reg) {
		printk("ioremap of slice reg fail.\n");
	}

	mailbox_out(("mb init OK!\n"));

	return MAILBOX_OK;
}

MAILBOX_EXTERN int mailbox_register_cb(unsigned int mailcode,
				       void (*cb) (void *mbuf, void *handle,
						   void *data),
				       void *usr_handle, void *usr_data)
{
	struct mb_cb *read_cb = MAILBOX_NULL;
	struct mb_buff *mbuf = MAILBOX_NULL;
	struct mb *mb = MAILBOX_NULL;
	unsigned int dst_id;

	mb = &g_mailbox_handle;

	dst_id = mailbox_get_dst_id(mailcode);

	printk("mb->local_id = %d dst_id = %d\n", mb->local_id, dst_id);

	if (mb->local_id != dst_id) {
		return mailbox_logerro_p1(MAILBOX_ERR_GUT_INVALID_TARGET_CPU,
					  mailcode);
	}

	mbuf = mailbox_get_channel_handle(mb, mailcode);
	if (MAILBOX_NULL == mbuf) {
		return mailbox_logerro_p1(MAILBOX_ERR_GUT_INVALID_CHANNEL_ID,
					  mailcode);
	}

	read_cb = &mbuf->read_cb[mailbox_get_use_id(mailcode)];
	if (MAILBOX_NULL != read_cb->func) {
		/*mailbox_logerro_p1(MAILBOX_WARNING_USER_CALLBACK_ALREADY_EXIST, mailcode); */
	}

	mailbox_mutex_lock(&mbuf->mutex);
	read_cb->handle = usr_handle;
	read_cb->data = usr_data;
	read_cb->func = cb;
	mailbox_mutex_unlock(&mbuf->mutex);

	return MAILBOX_OK;
}

MAILBOX_EXTERN int mailbox_init_usr_queue(struct mb_buff *mb_buf)
{
	unsigned int hsize = sizeof(struct mb_mail);
	struct mb_queue *usrqu = &mb_buf->usr_queue;

	mailbox_memcpy((void *)usrqu, (const void *)&mb_buf->mail_queue,
		       sizeof(struct mb_queue));
	if (hsize >
	    mailbox_queue_left(usrqu->rear, usrqu->front, usrqu->length)) {
		return mailbox_logerro_p1(MAILBOX_FULL, mb_buf->mailcode);
	}

	if (hsize + usrqu->front >= usrqu->base + usrqu->length) {
		usrqu->front = usrqu->front + hsize - usrqu->length;
	} else {
		usrqu->front = usrqu->front + hsize;
	}
	return MAILBOX_OK;
}

MAILBOX_EXTERN int mailbox_request_buff(unsigned int mailcode, void *mb_buf)
{
	struct mb *mailbox = MAILBOX_NULL;
	int ret_val = MAILBOX_OK;
	struct mb_buff **mbuf = (struct mb_buff **)mb_buf;

	mailbox = mailbox_get_mb();
	*mbuf = MAILBOX_NULL;
	if (MAILBOX_NULL == mailbox) {
		return mailbox_logerro_p1(MAILBOX_ERR_GUT_INPUT_PARAMETER,
					  mailcode);
	}

	if (mailbox->local_id != mailbox_get_src_id(mailcode)) {
		return mailbox_logerro_p1(MAILBOX_ERR_GUT_INVALID_SRC_CPU,
					  mailcode);
	}

	ret_val = mailbox_request_channel(mailbox, mbuf, mailcode);
	if (MAILBOX_OK == ret_val) {
		(*mbuf)->mb = mailbox;
		ret_val = mailbox_init_usr_queue(*mbuf);
		if (MAILBOX_OK == ret_val) {
			return MAILBOX_OK;
		} else {
			mailbox_release_channel(mailbox, *mbuf);
			*mbuf = MAILBOX_NULL;
			return ret_val;
		}
	} else {
		return ret_val;
	}
}

MAILBOX_EXTERN int mailbox_write_buff(struct mb_queue *queue,
				      char *data, unsigned int size)
{
	if ((size <=
	     mailbox_queue_left(queue->rear, queue->front, queue->length))
	    && (size + sizeof(struct mb_mail) <=
		mailbox_queue_left(queue->rear, queue->front, queue->length))
	    ) {
		return mailbox_queue_write(queue, (char *)data, size);
	}
	return 0;
}

MAILBOX_EXTERN int mailbox_read_buff(struct mb_queue *queue,
				     char *data, unsigned int size)
{
	return mailbox_queue_read(queue, (char *)data, size);
}

MAILBOX_EXTERN int mailbox_sealup_buff(struct mb_buff *mb_buf,
				       unsigned int usr_size)
{

	struct mb_mail mail = { 0 };
	struct mb_queue *m_queue = MAILBOX_NULL;
	unsigned int time_stamp;
	unsigned long mail_addr;

	m_queue = &mb_buf->mail_queue;

	if (usr_size > (mb_buf->config->single_max - sizeof(struct mb_mail))) {
		return (int)
		    mailbox_logerro_p2(MAILBOX_ERR_GUT_WRITE_EXCEED_MAX_SIZE,
				       usr_size, mb_buf->mailcode);
	} else {
	}

	time_stamp = (unsigned int)mailbox_get_timestamp();
	mail.ulPartition = MAILBOX_MSGHEAD_NUMBER;
	mail.ulWriteSlice = time_stamp;
	mail.ulReadSlice = 0;
	mail.ulPriority = 0;
	mail.ulSeqNum = mb_buf->seq_num;
	mail.ulMsgLength = usr_size;
	mail.ulMailCode = mb_buf->mailcode;

	if (MAILBOX_SEQNUM_START == mb_buf->seq_num) {
		(void)mailbox_logerro_p1(MAILBOX_INFO_SEND_FIRST_MAIL,
					 mb_buf->mailcode);
	}

	mb_buf->seq_num++;

	mail_addr = m_queue->front;
	mailbox_queue_write(m_queue, (char *)(&mail), sizeof(struct mb_mail));

	m_queue->front =
	    mailbox_align_size(mb_buf->usr_queue.front, MAILBOX_ALIGN);

	mailbox_flush_buff(mb_buf);

	mailbox_record_send(&(mb_buf->mntn), mb_buf->mailcode, time_stamp,
			    mail_addr);

	return mailbox_init_usr_queue(mb_buf);
}

MAILBOX_EXTERN int mailbox_flush_buff(struct mb_buff *mbuff)
{
	struct mb_head *head = MAILBOX_NULL;
	struct mb_queue *queue = MAILBOX_NULL;
	unsigned int channel_id = mbuff->channel_id;
	struct mb *mb = mbuff->mb;
	head = (struct mb_head *)mbuff->config->head_addr;
	queue = &mbuff->mail_queue;

	if (mb->local_id == mailbox_get_src_id(channel_id)) {
		head->ulFront =
		    (unsigned int)(queue->front -
				   queue->base) / sizeof(unsigned int);
		head->ulFrontslice = (unsigned int)mailbox_get_timestamp();

	} else if (mb->local_id == mailbox_get_dst_id(channel_id)) {
		head->ulRear =
		    (unsigned int)(queue->rear -
				   queue->base) / sizeof(unsigned int);
		head->ulRearslice = (unsigned int)mailbox_get_timestamp();
	}

	return MAILBOX_OK;
}

MAILBOX_EXTERN int mailbox_send_buff(struct mb_buff *mbuf)
{
	return mailbox_delivery(mbuf->channel_id);
}

MAILBOX_EXTERN int mailbox_release_buff(struct mb_buff *mbuf)
{
	return mailbox_release_channel(mbuf->mb, mbuf);
}

MAILBOX_EXTERN unsigned long mailbox_virt_to_phy(unsigned long virt_addr)
{
	unsigned long offset = 0;
	if (NULL == g_shareMemBase) {
		g_shareMemBase =
		    ioremap(MAILBOX_MEM_BASEADDR, MAILBOX_MEM_LENGTH);
	}
	offset = (unsigned long)g_shareMemBase - MAILBOX_MEM_BASEADDR;

	return (virt_addr - offset);
}

MAILBOX_EXTERN unsigned long mailbox_phy_to_virt(unsigned long phy_addr)
{
	unsigned long offset = 0;
	if (NULL == g_shareMemBase) {
		g_shareMemBase =
		    ioremap(MAILBOX_MEM_BASEADDR, MAILBOX_MEM_LENGTH);
	}
	offset = (unsigned long)g_shareMemBase - MAILBOX_MEM_BASEADDR;

	return (phy_addr + offset);
}
