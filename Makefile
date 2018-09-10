.PHONY: deps world docs serve clean cleanall bench test

NODE_BIN=./node_modules/.bin
BSB=$(NODE_BIN)/bsb

deps:
	@yarn

docs: build
	@./scripts/mk-docs.sh

spec-docs:
	@./scripts/setup-tla-plus.sh
	@./scripts/tla-to-pdf.sh

build: deps
	@$(BSB) -make-world

world: deps
	@$(BSB) -make-world -w

bench:
	@node lib/js/bench/Bench.bs.js

test:
	@node lib/js/test/Test.bs.js

serve: deps
	@$(NODE_BIN)/static

clean:
	@$(BSB) -clean-world

cleanall: clean
	@rm -rf node_modules lib
