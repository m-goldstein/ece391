# interrupt_linkage.S - Assembly linkage for interrupt handlers
# vim:ts=4 noexpandtab

.text
.globl keyboard_linkage, rtc_linkage, pit_linkage

keyboard_linkage:
  pushal
  call keyboard_handler
  popal
  iret

rtc_linkage:
  pushal
  call rtc_handler
  popal
  iret

pit_linkage:
  pushal
  call pit_handler
  popal
  iret
