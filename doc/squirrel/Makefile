# Regenerate squirrel.1 from squirrel.md using pandoc.
#
# Usage:
#   make man        - regenerate squirrel.1 from squirrel.md
#   make view       - render man page in the terminal
#   make clean      - remove generated squirrel.1
#
# squirrel.md is the single source of truth.
# Edit squirrel.md, then run 'make man' to update squirrel.1.
# Commit both files so squirrel.1 can be installed without pandoc.

PANDOC   ?= pandoc
MD       = squirrel.md
MAN      = squirrel.1

MAN_METADATA = \
    -M title="SQUIRREL" \
    -M section="1" \
    -M date="$(shell date '+%Y')" \
    -M header="Squirrel Data Package Utilities"

.PHONY: man view clean

man: $(MAN)

$(MAN): $(MD)
	$(PANDOC) $(MD) -s -t man $(MAN_METADATA) -o $(MAN)
	@echo "Generated $(MAN)"

view: $(MAN)
	man ./$(MAN)

clean:
	rm -f $(MAN)
