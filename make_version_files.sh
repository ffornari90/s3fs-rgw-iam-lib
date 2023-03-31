#!/bin/sh
#----------------------------------------------------------
# Directories
#----------------------------------------------------------
#PRGNAME=$(basename "$0")
SCRIPTDIR=$(dirname "$0")

CHANGELOG_FILE="${SCRIPTDIR}/CHANGELOG.md"
COMMIT_HASH_FILE="${SCRIPTDIR}/commit_hash"
PRODUCT_VERSION_FILE="${SCRIPTDIR}/product_version"

if [ -z "$1" ] || [ "$1" = "commit_hash" ]; then
	IS_COMMIT_HASH=1
elif [ "$1" = "product_version" ]; then
	IS_COMMIT_HASH=0
else
	printf "%s" "ERROR"
fi

#----------------------------------------------------------
# git command and .git directory
#----------------------------------------------------------
USE_GIT=0
if command -v git >/dev/null 2>&1; then
	if test -d "${SCRIPTDIR}"/.git; then
		USE_GIT=1
	fi
fi

#----------------------------------------------------------
# Create Git Commit Hash file
#----------------------------------------------------------
if [ "${IS_COMMIT_HASH}" -eq 1 ]; then
	SHORT_COMMIT_HASH=""

	if [ "${USE_GIT}" -eq 0 ]; then
		SHORT_COMMIT_HASH="unknown"
	else
		cd "${SCRIPTDIR}"
		SHORT_COMMIT_HASH=$(git rev-parse --short HEAD | tr -d '\n')
	fi

	printf "%s" "${SHORT_COMMIT_HASH}" > "${COMMIT_HASH_FILE}"
	printf "Commit Hash = %s\n" "${SHORT_COMMIT_HASH}"

else
#----------------------------------------------------------
# Create Version string file
#----------------------------------------------------------
	PRODUCT_VERSION_NUMBER=""

	CHANGELOG_VERSION=$(grep '^## \[' "${CHANGELOG_FILE}" 2>/dev/null | sed -e 's/^## \[//g' -e 's/\] - .*$//g' | tr -d '\n')

	if [ "${USE_GIT}" -eq 0 ]; then
		if [ -n "${CHANGELOG_VERSION}" ]; then
			PRODUCT_VERSION_NUMBER="${CHANGELOG_VERSION}"
		else
			PRODUCT_VERSION_NUMBER="0.0.0"
		fi
	else
		cd "${SCRIPTDIR}"

		GIT_TAG_VERSION=$(git tag --list | grep '^v' | sort | tail -1 | sed -e 's/^v//g' | tr -d '\n')

		if [ -z "${GIT_TAG_VERSION}" ] && [ -z "${CHANGELOG_VERSION}" ]; then
			PRODUCT_VERSION_NUMBER="0.0.0"
		elif [ -z "${GIT_TAG_VERSION}" ]; then
			PRODUCT_VERSION_NUMBER="${CHANGELOG_VERSION}"
		elif [ -z "${CHANGELOG_VERSION}" ]; then
			PRODUCT_VERSION_NUMBER="${GIT_TAG_VERSION}"
		else
			GIT_TAG_VERSION_MAJOR=$(echo "${GIT_TAG_VERSION}" | awk -F '.' '{print $1}')
			GIT_TAG_VERSION_MINOR=$(echo "${GIT_TAG_VERSION}" | awk -F '.' '{print $2}')
			GIT_TAG_VERSION_PATCH=$(echo "${GIT_TAG_VERSION}" | awk -F '.' '{print $3}')

			CHANGELOG_VERSION_MAJOR=$(echo "${CHANGELOG_VERSION}" | awk -F '.' '{print $1}')
			CHANGELOG_VERSION_MINOR=$(echo "${CHANGELOG_VERSION}" | awk -F '.' '{print $2}')
			CHANGELOG_VERSION_PATCH=$(echo "${CHANGELOG_VERSION}" | awk -F '.' '{print $3}')

			if [ "${GIT_TAG_VERSION_MAJOR}" -gt "${GIT_TAG_VERSION_MAJOR}" ]; then
				PRODUCT_VERSION_NUMBER="${GIT_TAG_VERSION}"
			elif [ "${GIT_TAG_VERSION_MAJOR}" -lt "${GIT_TAG_VERSION_MAJOR}" ]; then
				PRODUCT_VERSION_NUMBER="${CHANGELOG_VERSION}"
			else
				if [ "${GIT_TAG_VERSION_MINOR}" -gt "${GIT_TAG_VERSION_MINOR}" ]; then
					PRODUCT_VERSION_NUMBER="${GIT_TAG_VERSION}"
				elif [ "${GIT_TAG_VERSION_MINOR}" -lt "${GIT_TAG_VERSION_MINOR}" ]; then
					PRODUCT_VERSION_NUMBER="${CHANGELOG_VERSION}"
				else
					if [ "${GIT_TAG_VERSION_PATCH}" -gt "${GIT_TAG_VERSION_PATCH}" ]; then
						PRODUCT_VERSION_NUMBER="${GIT_TAG_VERSION}"
					else
						PRODUCT_VERSION_NUMBER="${CHANGELOG_VERSION}"
					fi
				fi
			fi
		fi
	fi

	printf "%s" "${PRODUCT_VERSION_NUMBER}" > "${PRODUCT_VERSION_FILE}"
	printf "Product Version = %s\n" "${PRODUCT_VERSION_NUMBER}"
fi
exit 0
