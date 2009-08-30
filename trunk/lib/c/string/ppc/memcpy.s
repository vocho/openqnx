#
# $QNXLicenseA:
# Copyright 2007, QNX Software Systems. All Rights Reserved.
# 
# You must obtain a written license from and pay applicable license fees to QNX 
# Software Systems before you may reproduce, modify or distribute this software, 
# or any work that includes all or part of this software.   Free development 
# licenses are available for evaluation and non-commercial purposes.  For more 
# information visit http://licensing.qnx.com or email licensing@qnx.com.
#  
# This file may contain contributions from others.  Please review this entire 
# file for other proprietary rights or license notices, as well as the QNX 
# Development Suite License Guide at http://licensing.qnx.com/license-guide/ 
# for other information.
# $
#


	.text
        .align          2                  
        .globl          memcpy             #visible to linker

memcpy:
        cmplwi          %cr0, %r5, 0
        addi            %r12, %r3, 0       #save gpr 3 for %return
   		beqlr-          %cr0               #0 byte to copy, done!
		
		li				%r10,1

		# fix target address not in word boundary to max performance

        rlwinm.         %r7, %r3, 0, 3     #r7 = extract bits 30-31
        subfic          %r7, %r7, 0x4      #calculate how many bytes will
                                           #reach next word
        cmplw           %cr1, %r5, %r7     #r5 <= %r7?
        beq             %cr0, ..do2        #r7 == 00, ok no boundary problem
        ble             %cr1, ..do3

		# 1 <= r7 <= 3
		sub				%r5,%r5,%r7			#update copy length
		mr				%r6,%r7
		sub.			%r7,%r7,%r10		# r10 == 1
		lbz				%r0,0(%r4)
		stb				%r0,0(%r12)
		beq				1f
		sub.			%r7,%r7,%r10		# r10 == 1
		lbz				%r8,1(%r4)
		stb				%r8,1(%r12)
		beq				1f
		lbz				%r0,2(%r4)
		stb				%r0,2(%r12)
1:
		add				%r4,%r4,%r6
		add				%r12,%r12,%r6

..do2:
		rlwinm.         %r6, %r5, 0x1c, 0xfffffff  #r6 = no of byte / 16
        mtctr           %r6    		       #copy %r6 to ctr %register
        beq             %cr0, ..do3

1:
		lwz				%r6,0(%r4)
		lwz				%r7,4(%r4)
		lwz				%r8,8(%r4)
		lwz				%r9,12(%r4)
        addi            %r4,%r4,0x10		#bump 16 bytes
		stw				%r6,0(%r12)
		stw				%r7,4(%r12)
		stw				%r8,8(%r12)
		stw				%r9,12(%r12)
        addi            %r12,%r12,0x10
        bdnz            1b					#if ctr != 0, then loop again

..do3:  
		# handle the tail end of the copy
		andi.	    %r5,%r5,0xf  #no of bytes < 16

		beqlr		
		sub.		%r5,%r5,%r10	# r10 == 1
		lbz			%r0,0(%r4)
		stb			%r0,0(%r12)
		beqlr		
		sub.		%r5,%r5,%r10	# r10 == 1
		lbz			%r6,1(%r4)
		stb			%r6,1(%r12)
		beqlr		
		sub.		%r5,%r5,%r10	# r10 == 1
		lbz			%r7,2(%r4)
		stb			%r7,2(%r12)
		beqlr		
		sub.		%r5,%r5,%r10	# r10 == 1
		lbz			%r8,3(%r4)
		stb			%r8,3(%r12)
		beqlr		
		sub.		%r5,%r5,%r10	# r10 == 1
		lbz			%r9,4(%r4)
		stb			%r9,4(%r12)
		beqlr		
		sub.		%r5,%r5,%r10	# r10 == 1
		lbz			%r0,5(%r4)
		stb			%r0,5(%r12)
		beqlr		
		sub.		%r5,%r5,%r10	# r10 == 1
		lbz			%r6,6(%r4)
		stb			%r6,6(%r12)
		beqlr		
		sub.		%r5,%r5,%r10	# r10 == 1
		lbz			%r7,7(%r4)
		stb			%r7,7(%r12)
		beqlr		
		sub.		%r5,%r5,%r10	# r10 == 1
		lbz			%r8,8(%r4)
		stb			%r8,8(%r12)
		beqlr		
		sub.		%r5,%r5,%r10	# r10 == 1
		lbz			%r9,9(%r4)
		stb			%r9,9(%r12)
		beqlr		
		sub.		%r5,%r5,%r10	# r10 == 1
		lbz			%r0,10(%r4)
		stb			%r0,10(%r12)
		beqlr		
		sub.		%r5,%r5,%r10	# r10 == 1
		lbz			%r6,11(%r4)
		stb			%r6,11(%r12)
		beqlr		
		sub.		%r5,%r5,%r10	# r10 == 1
		lbz			%r7,12(%r4)
		stb			%r7,12(%r12)
		beqlr		
		sub.		%r5,%r5,%r10	# r10 == 1
		lbz			%r8,13(%r4)
		stb			%r8,13(%r12)
		beqlr		
		sub.		%r5,%r5,%r10	# r10 == 1
		lbz			%r9,14(%r4)
		stb			%r9,14(%r12)
		beqlr		
		lbz			%r0,15(%r4)
		stb			%r0,15(%r12)

        blr

        .type memcpy,@function
        .size memcpy,.-memcpy
