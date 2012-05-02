
RESWRAP  = reswrap

ICONS= \
 cy_ecom_s.jpg \
 cy_ecom_n.jpg \
 cy_ecom_b.jpg \
 cy_ecom_plus_s.jpg \
 cy_ecom_plus_n.jpg \
 cy_ecom_plus_b.jpg \
 cy_pinpad_s.jpg \
 cy_pinpad_n.jpg \
 cy_pinpad_b.jpg \
 cy_secoder_s.jpg \
 cy_secoder_n.jpg \
 cy_secoder_b.jpg

EXTRA_DIST=\
 $(ICONS) icons.cpp \
 cy_ecom_0.jpg \
 cy_pinpad_0.jpg \
 cy_secoder_0.jpg \
 cy_ecom_plus_0.jpg
 

icons.cpp: $(ICONS)
	$(RESWRAP) -p "fxcj_icon_" -o icons.cpp $^
