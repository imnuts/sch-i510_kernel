#ifndef _SVNET_CIRC_BUF_H_
#define _SVNET_CIRC_BUF_H_

#define CIRC_CNT(head,tail,size) ((head) < (tail) ? \
		(head) + (size) - (tail) : (head) - (tail))
#define CIRC_SPACE(head,tail,size) CIRC_CNT((tail),((head)+1),(size))
#define CIRC_CNT_TO_END(head,tail,size) ((head) < (tail) ? \
		(size) - (tail) : (head) - (tail))
#define CIRC_SPACE_TO_END(head,tail,size) ((head) < (tail) ? \
		(tail) - (head) - 1 : (size) - (head) - !(tail))

#endif /* _SVNET_CIRC_BUF_H_ */
