echo '['
git rev-list --ancestry-path $1..$2 | while read sha1; do
    full_diff="$(git show --format='' $sha1 | sed 's/\"/\\\"/g')"
    git --no-pager show --format="{%n  \"commit_hash\": \"%H\",%n  \"author\": \"%an\",%n  \"date\": \"%ad\",%n  \"commit_message\": '%s',%n }," -s $sha1
    done
echo ']'