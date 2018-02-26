#!/bin/bash
set -euo pipefail

if [[ -z "${GH_TOKEN:-}" ]]; then
	echo "GH_TOKEN not set!"
	exit 1
fi

master_commit=$TRAVIS_COMMIT

result_file=(build/Testing/*/Test.xml)

passed=$(grep -c 'Status="passed"' "$result_file")
failed=$(grep -c 'Status="failed"' "$result_file")

git checkout travis_results
git remote add pushable "https://${GH_TOKEN}@github.com/thomasmoelhave/tpie.git"

try_push() {
	if ! git push pushable travis_results; then
		# Retry
		update
		exit 0
	fi
}

update() {
	git fetch origin travis_results
	git reset --hard origin/travis_results

	result_commit=$(cat commit)

	if [[ "$master_commit" != "$result_commit" ]]; then
		if [[ "$(git cat-file -t "$result_commit" 2>/dev/null || true)" == "commit" ]]; then
			# This commit exists so we are the newer one
			echo -n "$master_commit" > commit
			echo -n 0 > passed
			echo -n 0 > failed

			git add commit passed failed
			git commit -m 'Update count'
			try_push
		else
			# This commit doesn't exist, so this is probably an old build, exit
			exit 0
		fi
	fi

	# Same commit, update
	let passed+=$(cat passed)
	let failed+=$(cat failed)

	echo -n "$passed" > passed
	echo -n "$failed" > failed

	if [[ "$failed" -eq 0 ]]; then
		text="$passed succeeded"
		color="green"
	else
		text="$passed succeeded, $failed failed"
		color="orange"
	fi

	curl "https://img.shields.io/badge/tests-$text-$color.svg" -o badge.svg

	git add passed failed badge.svg
	git commit -m 'Update count'
	try_push
}

update
