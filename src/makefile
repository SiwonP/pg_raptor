MODULE_big = pg_raptor
EXTENSION = pg_raptor     # the extersion's name
DATA = pg_raptor--0.0.1.sql    # script file to install
OBJS = pg_raptor.o raptor_fetch.o raptor_utils.o raptor_steps.o interval_utils.o
# REGRESS = get_sum_test      # the test script file

# for posgres build
PG_CONFIG = pg_config
PGXS := $(shell $(PG_CONFIG) --pgxs)
include $(PGXS)