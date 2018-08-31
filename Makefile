.PHONY: deps world docs serve clean cleanall

deps:
	@yarn

docs:
	@./scripts/setup-tla-plus.sh
	@./scripts/tla-to-pdf.sh

world: deps
	@./node_modules/.bin/bsb -make-world -w

serve: deps
	@./node_modules/.bin/static

clean:
	@./node_modules/.bin/bsb -clean-world

cleanall: clean
	@rm -rf node_modules lib
