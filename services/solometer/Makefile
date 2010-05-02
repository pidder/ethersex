TOPDIR ?= ../..
include $(TOPDIR)/.config

$(SOLOMETER_SUPPORT)_SRC += services/solometer/pv_serial.c
$(SOLOMETER_SUPPORT)_SRC += services/solometer/pv_inet.c
$(SOLOMETER_SUPPORT)_SRC += services/solometer/pv_measure.c
$(WR_EFFEKTA)_SRC += services/solometer/pv_effekta.c
$(WR_SMA)_SRC += services/solometer/pv_sma.c
$(SOLOMETER_SOAP_SUPPORT)_SRC += services/solometer/pv_soap.c

#embed/pvs.ht.m4	:	services/solometer/soap/m4header services/solometer/soap/pvs.ht
#	cat services/solometer/soap/m4header services/solometer/soap/pvs.ht >embed/pvs.ht.m4
#
#embed/scl.js.m4	:	services/solometer/soap/m4header services/solometer/soap/scl.js
#	cat services/solometer/soap/m4header services/solometer/soap/scl.js >embed/scl.js.m4

##############################################################################
# generic fluff
include $(TOPDIR)/scripts/rules.mk
