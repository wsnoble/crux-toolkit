# file      : build/c/c-o.make
# author    : Boris Kolpackov <boris@codesynthesis.com>
# copyright : Copyright (c) 2004-2011 Code Synthesis Tools CC
# license   : GNU GPL v2; see accompanying LICENSE file

$(call include,$(bld_root)/c/configuration.make)


ifdef c_id
$(call include-once,$(bld_root)/c/$(c_id)/c-o.make,$(out_base))
endif
