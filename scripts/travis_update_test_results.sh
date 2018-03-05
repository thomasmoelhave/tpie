#!/bin/bash
set -euo pipefail

if [[ "$TRAVIS_BRANCH" != "travis" || "$TRAVIS_EVENT_TYPE" == "pull_request" ]]; then
	echo "Skipping updating test status, branch: $TRAVIS_BRANCH, event type: $TRAVIS_EVENT_TYPE"
	exit 0
fi

if [[ -z "${GH_TOKEN:-}" ]]; then
	echo "GH_TOKEN not set!"
	exit 1
fi

current_build_number=$TRAVIS_BUILD_NUMBER
echo "Current build number: $current_build_number"

result_file=(build/Testing/*/Test.xml)

passed=$(grep -c 'Status="passed"' "$result_file" || echo 0)
failed=$(grep -c 'Status="failed"' "$result_file" || echo 0)

cd /tmp
git clone "https://${GH_TOKEN}@github.com/Tyilo/tpie-travis-results.git"
cd tpie-travis-results

try_push() {
	if ! git push; then
		# Retry
		echo "Failed to push, retrying in a bit..."
		sleep $(($RANDOM % 15 + 10))

		update
		exit 0
	fi
}

update() {
	git fetch origin master:origin/master
	git reset --hard origin/master

	result_build_number=$(cat build_number || echo -1)

	if [[ "$current_build_number" -lt "$result_build_number" ]]; then
		echo "Found newer build number: $current_build_number < $result_build_number"
		exit 0
	fi

	if [[ "$current_build_number" -gt "$result_build_number" ]]; then
		echo "Newer build, changing build number in travis_results"

		echo -n "$current_build_number" > build_number
		echo -n 0 > passed
		echo -n 0 > failed

		git add build_number passed failed
		git commit -m "Build $current_build_number"
		try_push
	fi

	# Same build number, update
	echo "Updating counts"

	let passed+=$(cat passed || echo 0) || true
	let failed+=$(cat failed || echo 0) || true

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
