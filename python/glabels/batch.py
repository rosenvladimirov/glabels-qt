"""Subprocess-based fallback for batch operations using glabels-batch-qt."""

import subprocess
import shutil


def find_glabels_batch():
    """Find the glabels-batch-qt executable."""
    path = shutil.which("glabels-batch-qt")
    if path is None:
        path = shutil.which("glabels-batch")
    return path


def render_pdf(
    glabels_file,
    output="output.pdf",
    merge_input=None,
    copies=None,
    sheets=None,
    first=None,
    outlines=False,
    crop_marks=False,
    reverse=False,
    collate=False,
    group=False,
    variables=None,
    executable=None,
):
    """Render a .glabels file to PDF using glabels-batch-qt subprocess.

    This is a zero-dependency fallback that does not require the C++ bindings.

    Args:
        glabels_file: Path to the .glabels project file.
        output: Output PDF filename (default: "output.pdf").
        merge_input: Merge data source file path.
        copies: Number of copies.
        sheets: Number of full sheets.
        first: Starting label position (1-based).
        outlines: Print label outlines.
        crop_marks: Print crop marks.
        reverse: Print in reverse (mirror).
        collate: Collate merge copies.
        group: Start each merge group on a new page.
        variables: Dict of variable definitions {name: value}.
        executable: Path to glabels-batch-qt (auto-detected if None).

    Returns:
        CompletedProcess result.

    Raises:
        FileNotFoundError: If glabels-batch-qt is not found.
        subprocess.CalledProcessError: If the command fails.
    """
    if executable is None:
        executable = find_glabels_batch()
    if executable is None:
        raise FileNotFoundError(
            "glabels-batch-qt not found. Install gLabels or specify the path."
        )

    cmd = [executable]

    if output:
        cmd.extend(["-o", output])
    if merge_input:
        cmd.extend(["-i", merge_input])
    if copies is not None:
        cmd.extend(["-c", str(copies)])
    if sheets is not None:
        cmd.extend(["-s", str(sheets)])
    if first is not None:
        cmd.extend(["-f", str(first)])
    if outlines:
        cmd.append("-l")
    if crop_marks:
        cmd.append("-m")
    if reverse:
        cmd.append("-r")
    if collate:
        cmd.append("-a")
    if group:
        cmd.append("-g")
    if variables:
        for name, value in variables.items():
            cmd.extend(["-D", f"{name}={value}"])

    cmd.append(glabels_file)

    return subprocess.run(cmd, check=True, capture_output=True, text=True)
