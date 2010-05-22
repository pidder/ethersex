TOPDIR ?= ../..
include $(TOPDIR)/.config

$(SOLOMETER_SUPPORT)_SRC += services/solometer/pv_serial.c
$(SOLOMETER_SUPPORT)_SRC += services/solometer/pv_inet.c
$(SOLOMETER_SUPPORT)_SRC += services/solometer/pv_measure.c
$(PV_WR_EFFEKTA)_SRC += services/solometer/pv_effekta.c
$(PV_WR_SMA)_SRC += services/solometer/pv_sma.c
$(SOLOMETER_HTTP_CONFIG)_SRC += services/solometer/http_config/handle_solometer.c

#services/solometer/server/upload_data.php : $(TOPDIR)/.config
serversoft:
	sed s/MeinKeks/$(PV_SOLOMETER_ID)/ services/solometer/server/upload_template.php | \
	  sed s/RawFilesKept/$(PV_KEEP_RAW)/ \
	  >services/solometer/server/upload_data.php
	cat services/solometer/soap/m4header services/solometer/soap/pvs.ht >embed/pvs.ht.m4

#embed/pvs.ht.m4	:	services/solometer/soap/m4header services/solometer/soap/pvs.ht
#	cat services/solometer/soap/m4header services/solometer/soap/pvs.ht >embed/pvs.ht.m4

#embed/scl.js.m4	:	services/solometer/soap/m4header services/solometer/soap/scl.js
#	cat services/solometer/soap/m4header services/solometer/soap/scl.js >embed/scl.js.m4

##############################################################################
# generic fluff
include $(TOPDIR)/scripts/rules.mk
