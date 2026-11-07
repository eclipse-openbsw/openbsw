import json
import os
import sys
from urllib.parse import urlencode
from urllib.request import Request, urlopen


def gh_get_json(url: str, token: str) -> object:
    req = Request(
        url,
        headers={
            "Authorization": f"Bearer {token}",
            "Accept": "application/vnd.github+json",
        },
        method="GET",
    )

    with urlopen(req) as response:
        charset = response.headers.get_content_charset() or "utf-8"
        return json.loads(response.read().decode(charset))


def main() -> int:
    token = os.environ.get("GITHUB_TOKEN")
    repo = os.environ.get("REPO")
    sha = os.environ.get("SHA")
    out_path = os.environ.get("GITHUB_OUTPUT")

    if not token or not repo or not sha or not out_path:
        missing = [
            k
            for k, v in {
                "GITHUB_TOKEN": token,
                "REPO": repo,
                "SHA": sha,
                "GITHUB_OUTPUT": out_path,
            }.items()
            if not v
        ]
        print(f"Missing required env vars: {', '.join(missing)}", file=sys.stderr)
        return 2

    page = 1
    max_pages = 20
    found = None

    for _ in range(max_pages):
        params = urlencode({"state": "open", "per_page": 100, "page": page})
        url = f"https://api.github.com/repos/{repo}/pulls?{params}"

        prs = gh_get_json(url, token)
        if not isinstance(prs, list):
            print(f"Unexpected response (not a list) from {url}", file=sys.stderr)
            return 3

        if not prs:
            break

        for pr in prs:
            try:
                if pr["head"]["sha"] == sha:
                    found = pr
                    break
            except (TypeError, KeyError):
                continue

        if found:
            break

        page += 1

    if not found:
        print(f"No open PR found with head.sha={sha}", file=sys.stderr)
        return 1

    number = found["number"]
    head_ref = found["head"]["ref"]
    base_ref = found["base"]["ref"]

    if not number or not head_ref or not base_ref:
        print(f"Could not parse the PR data")
        return 1

    with open(out_path, "a", encoding="utf-8") as f:
        f.write(f"number={number}\n")
        f.write(f"head_ref={head_ref}\n")
        f.write(f"base_ref={base_ref}\n")

    return 0


if __name__ == "__main__":
    sys.exit(main())
