.PHONY: test
TESTDIR=./test
BUILDDIR=$(TESTDIR)/build
FIXTURESDIR=$(TESTDIR)/fixtures
OPTS=-std=c++1z -lpthread
TMP=./tmp.txt
CMP=cmp --silent $(TMP)
ERROR=echo [error]
RM=rm $(TMP)
FILES=`ls $(TESTDIR)/*.cc`
MSG="program has failed to output a value matching the fixure"

Test = \
	echo "[info] building... $(1)"; \
	clang++ $(OPTS) -o $(BUILDDIR)/$(1) $(TESTDIR)/$(1).cc; \
	$(BUILDDIR)/$(1) > $(TMP); \
	echo "[info] comparing output to fixture '$(FIXTURESDIR)/$(1).txt'"; \
	$(CMP) $(FIXTURESDIR)/$(1).txt || $(ERROR) $(1) $(MSG) $(FIXTURESDIR)/$(1).txt \
	$(RM)

build:
	@for f in $(FILES); do \
		echo "[info] building... $$f"; \
		clang++ $(OPTS) $$f -o $(BUILDDIR)/`basename "$$f" .cc`; \
	done 

ifeq (test,$(firstword $(MAKECMDGOALS)))
  RUN_ARGS := $(wordlist 2,$(words $(MAKECMDGOALS)),$(MAKECMDGOALS))
  $(eval $(RUN_ARGS):;@:)
endif

ifeq ($(words $(RUN_ARGS)), 0)
  RUN_ARGS := "all"
endif

test:
ifeq ($(RUN_ARGS),"all")
	@for f in $(FILES); do \
		$(call Test,`basename "$$f" .cc`); \
	done
else
	@$(call Test,`basename "$(RUN_ARGS)" .cc`);
endif
	@$(RM)

