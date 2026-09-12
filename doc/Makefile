# squirrel man page tooling.
#
# BOTH squirrel.md and squirrel.1 are hand-maintained and kept in sync by hand.
# squirrel.1 is NOT auto-generated on commit: it carries manual groff tuning
# (grouped .BR option lists, .RS blocks) that pandoc does not reproduce. When
# you change the CLI, edit squirrel.md and squirrel.1 together and commit both.
#
# Usage:
#   make draft      - render squirrel.md to squirrel.1.draft via pandoc, as a
#                     STARTING POINT only. Never overwrites the real squirrel.1;
#                     diff it in, then re-apply the manual formatting by hand.
#   make view       - render the committed squirrel.1 in the terminal
#   make clean      - remove the throwaway squirrel.1.draft (not squirrel.1)
#
# 'make draft' requires pandoc; the hand-maintained squirrel.1 does not.

PANDOC   ?= pandoc
MD       = squirrel.md
MAN      = squirrel.1
DRAFT    = squirrel.1.draft

MAN_METADATA = \
    -M title="SQUIRREL" \
    -M section="1" \
    -M date="$(shell date '+%Y')" \
    -M header="Squirrel Data Package Utilities"

.PHONY: draft view clean

draft: $(DRAFT)

$(DRAFT): $(MD)
	$(PANDOC) $(MD) -s -t man $(MAN_METADATA) -o $(DRAFT)
	@echo "Generated $(DRAFT) (reference only; do not commit as $(MAN))"

view: $(MAN)
	man ./$(MAN)

clean:
	rm -f $(DRAFT)
