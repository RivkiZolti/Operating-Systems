#include <stdio.h>
#include "os.h"
uint64_t check_mipuy(int index,uint64_t *pt){
	uint64_t* x=pt+(8*index);
	int mask=1;
	int a;
	a=mask & (*x);
	if(a==0){
	return 0;
	}else{
		return *x;
	}
}
uint64_t mipuy(int index,uint64_t *pt){
	uint64_t y;
	uint64_t *x=pt+(index*8);
	y=*x>>12;
	y=alloc_page_frame();
	int mask=1;
	y=y | mask;
	return y;
	
}

void page_table_update(uint64_t pt,uint64_t vpn,uint64_t ppn){
	pt=pt>>12;
	int i=4;
	int d;
	uint64_t f;
	uint64_t *e;
	uint64_t *b;
	uint64_t pt1;
	if (ppn==NO_MAPPING){
	    b=phys_to_virt(pt);
		while(i>0){
			f=(0x1ff<<(i*9))&vpn;
			d=f>>i*9;
			pt1=check_mipuy(d,b);
			if(!pt1){
				return;
			}
			e=phys_to_virt(pt1);
			b=e;
			i--;
		}
		f=(0x1ff<<(i*9))&vpn;
		d=f>>i*9;
		*(b+8*d)=*(b+8*d) | 1;
		*(b+8*d)=*(b+8*d) ^1;
	}
	else{
		uint64_t mask1=0x1ff;
		i=4;
		b=phys_to_virt(pt);
		while(i>0){
			f=(mask1<<(i*9))&vpn;
			d=f>>i*9;
			pt1=check_mipuy(d,b);
			
			if(pt1==0){
				pt1=(alloc_page_frame()<<12)|1;
				*(b+(8*d))=pt1;
			}
			e=phys_to_virt(pt1);
			
			b=e;
			i--;
		}
		f=mask1&vpn;
		d=f;
		*(b+(8*d))=(ppn<<12)|1;
		
}
}
uint64_t page_table_query(uint64_t pt, uint64_t vpn){
	pt=pt>>12;
	int i=4;
	uint64_t f;
	uint64_t *b;
	uint64_t *e;
	int d;
	uint64_t pt1;
	uint64_t mask1=0x1ff;
	b=phys_to_virt(pt);
	while(i>0){
		
		f=(mask1<<(i*9))&vpn;
		d=f>>i*9;
		pt1=check_mipuy(d,b);
		e=phys_to_virt(pt1);
		if((pt1 &1)==0){
		return NO_MAPPING;
		}
		b=e;
		i--;
	}
	f=mask1&vpn;
	d=f;
	uint64_t line_content=*(b+(8*d));
	if(!(line_content&1)){
		return NO_MAPPING;
	}
	return line_content>>12;
}
