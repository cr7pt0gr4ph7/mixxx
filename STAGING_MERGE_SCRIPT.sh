#!/bin/bash
# README: This script creates the contents of the 'staging' branch
#         from the feature branches plus the current main branch.
#
#         This script exists because the feature branches are developed
#         in parallel, but should be integrated into the main branch
#         in a linearized fashion.
#
# RERERE: The script assumes that https://git-scm.com/book/en/v2/Git-Tools-Rerere
#         is enabled to allow for automatic conflict resolution, but should also
#         work if it isn't. To enable it, run:
#
#         git config rerere.enabled true
#
#         Conflict resolutions will be recorded to .git/rr-cache
set -o errexit

case $MERGE_SCRIPT__IS_ISOLATED_COPY in
yes)
    echo "[Internal] Running as isolated copy of merge script from $(realpath "$0")"
    export MERGE_SCRIPT__IS_ISOLATED_COPY=
    ;;
*)
    # Copy the script to a temporary location and run it again
    export MERGE_SCRIPT__IS_ISOLATED_COPY="yes"
    MERGE_SCRIPT__ISOLATED_COPY_LOCATION=/tmp/staging_merge_script_$RANDOM
    MERGE_SCRIPT__ORIGINAL_LOCATION=$(realpath "$0")
    echo "[Internal] Copying merge script from ${MERGE_SCRIPT__ORIGINAL_LOCATION} to temporary location at ${MERGE_SCRIPT__ISOLATED_COPY_LOCATION}"
    cp "$MERGE_SCRIPT__ORIGINAL_LOCATION" "$MERGE_SCRIPT__ISOLATED_COPY_LOCATION"

    # Run from temporary location
    echo "[Internal] Running temporary copy of merge script"
    "$MERGE_SCRIPT__ISOLATED_COPY_LOCATION"
    # echo "FATAL: Failed to execute temporary copy of merge script"
    exit 0
    ;;
esac

function merge_branch() {
    local feature_branch=$1
    local branch_type=$2
    local source_branch=origin/$feature_branch
    local target_branch=staging
    local message_prefix=""
    local conflict_message_prefix=""

    case $branch_type in
    "")
        case $source_branch in
        origin/ready-for-merge/*)
            local branch_type=ready
            local message_prefix="[Preview] "
            local conflict_message_prefix="[Preview/Conflicts] "
            ;;
        origin/wip/*)
            local branch_type=wip
            local message_prefix="[WIP] "
            local conflict_message_prefix="[WIP/Conflicts] "
            ;;
        "")
            local branch_type=unknown
            local message_prefix="[Preview/WIP] "
            local conflict_message_prefix="[Preview/WIP/Conflicts] "
            ;;
        esac
        ;;

    ready-for-merge)
        local message_prefix="[Preview] "
        local conflict_message_prefix="[WIP/Conflicts] "
        ;;
    wip)
        local message_prefix="[WIP] "
        local conflict_message_prefix="[WIP/Conflicts] "
        ;;
    esac

    if git merge-base --is-ancestor "$source_branch" HEAD; then
        echo "[Skipped] $source_branch has already been merged into the target branch"
    else
        echo "[Merging] $source_branch (type: $branch_type) is being merged"
        local commit_message="Merge remote-tracking branch '$source_branch' into '$target_branch'"

        git merge --rerere-autoupdate -m "${message_prefix}$commit_message" "$source_branch" && git_merge_exitcode=$? || git_merge_exitcode=$?
        if [ "$git_merge_exitcode" -ne 0 ]; then
            # There was a conflict, but rerere might have already resolved it.
            # Check if the working directory is clean to see if that is the case.
            git diff --quiet && git_diff_exitcode=$? || git_diff_exitcode=$?
            if [ "$git_diff_exitcode" -eq 0 ]; then
                echo "It seems like all conflicts could be automatically resolved. Committing the result and continuing."
                git commit -m "${conflict_message_prefix}$commit_message"
            else
                echo "[Warning] Merge failed. Please resolve all merge conflicts, commit the result and rerun this script."
                exit 1
            fi
        fi
    fi
}

if [ -n "$(git status --porcelain)" ]; then
    # There are uncommitted changes. Let the user confirm that this is intentional
    # In practice, the only file that should have uncommitted changes is this merge script.
    # Everything else should be clean.
    echo ""
    git status
    echo ""

    while true; do
        read -r -p "[Warning] Your working directory contains changes. Do you still want to proceed? (y/n) " yn

        case $yn in
            [yY])
                echo "Proceeding..."
                break;;
            [nN])
                echo "Aborting...";
                exit;;
            * ) ;;
        esac
    done
fi

# This script assumes that we are on the most current version of the
# main branch. At the last update of this script that was commit
# 9893d7510cc60c5922672e1116829993619acbcd.
merge_branch configurable-edit-item-shortcut ready-for-merge
merge_branch autodj-time-remaining ready-for-merge
merge_branch ready-for-merge/fix-star-rating-editor
merge_branch ready-for-merge/sound-device-reload
merge_branch search-in-tracks-shortcut ready-for-merge
merge_branch ready-for-merge/track-info-dialog
merge_branch wip/better-beatspinbox-usability
merge_branch wip/more-hotcue-buttons
merge_branch wip/toast-notifications  # Conflicts expected
merge_branch wip/style-active-search-control
merge_branch ready-for-merge/autodj-search
