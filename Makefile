.PHONY: test
TESTDIR=./test
BUILDDIR=$(TESTDIR)/build
FIXTURESDIR=$(TESTDIR)/fixtures
OPTS=-std=c++1y -lpthread
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

all:
	@for f in $(FILES); do \
		echo "[info] building... $$f"; \
		clang++ $(OPTS) $$f -o $(BUILDDIR)/`basename "$$f" .cc`; \
	done 

test:
	@for f in $(FILES); do \
		$(call Test,`basename "$$f" .cc`); \
	done
	@$(RM)

