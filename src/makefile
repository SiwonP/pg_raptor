MODULE_big = raptor
EXTENSION = raptor     # the extersion's name
DATA = raptor--0.0.1.sql    # script file to install
OBJS = raptor.o raptor_fetch.o raptor_utils.o raptor_steps.o
# REGRESS = get_sum_test      # the test script file

# for posgres build
PG_CONFIG = pg_config
PGXS := $(shell $(PG_CONFIG) --pgxs)
include $(PGXS)