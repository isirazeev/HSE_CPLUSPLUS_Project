#!/usr/bin/env python3

import math
from pathlib import Path
import subprocess
import sys
import tempfile

from PIL import Image, ImageChops


# Имя, входной файл, аргументы, эталон/тип исключения, допуск (None — ожидается ошибка).
TEST_CASES = [
    ("flag_crop", "flag.bmp", ["-crop", "50", "50"], "flag_crop.bmp", 0.0),
    ("flag_gs", "flag.bmp", ["-gs"], "flag_gs.bmp", 1.0),
    ("flag_neg", "flag.bmp", ["-neg"], "flag_neg.bmp", 1.0),
    ("flag_edge", "flag.bmp", ["-edge", "0.1"], "flag_edge.bmp", 1.0),
    ("flag_edge_edge", "flag.bmp", ["-edge", "0.1", "-edge", "0.5"], "flag_edge_edge.bmp", 1.0),
    ("parrots_copy", "parrots.bmp", [], "parrots.bmp", 0.0),
    ("parrots_color", "parrots.bmp", ["-crop", "81", "53", "-neg", "-gs"], "parrots_color.bmp", 1.0),
    ("photo_blur", "photo.bmp", ["-blur", "3.0"], "photo_blur.bmp", 1.0),
    ("photo_sharp", "photo.bmp", ["-sharp"], "photo_sharp.bmp", 1.0),
    ("photo_hex", "photo.bmp", ["-hex", "192", "288", "3"], "photo_hex.bmp", 0.0),
    ("photo_hex_8", "photo.bmp", ["-hex", "192", "288", "8"], "photo_hex_8.bmp", 0.0),
    ("photo_spatial", "photo.bmp", ["-sharp", "-blur", "1.2", "-crop", "301", "451"], "photo_spatial.bmp", 1.0),
    ("photo_edges", "photo.bmp", ["-edge", "0.12", "-neg"], "photo_edges.bmp", 1.0),
    ("bad_bmp", "broken.bmp", [], "BMPError", None),
    ("bad_output", "flag.bmp", [], "FileError", None),
    ("bad_filter", "flag.bmp", ["-unknown"], "IncorrectFilter", None),
    ("bad_hex_radius", "photo.bmp", ["-hex", "192", "288", "0"], "IncorrectFilter", None),
    ("bad_argument", "flag.bmp", ["-crop", "abc", "1"], "InvalidArgument", None),
    ("bad_range", "flag.bmp", ["-crop", "9999999999999999999999999999999999999999", "1"], "OutOfRange", None),
]


def calc_images_distance(expected_path, actual_path):
    with Image.open(expected_path) as expected, \
            Image.open(actual_path) as actual:

        if expected.size != actual.size:
            raise ValueError(
                f"Image sizes differ: "
                f"expected {expected.size}, got {actual.size}"
            )

        diff = ImageChops.difference(
            expected.convert("RGB"),
            actual.convert("RGB"),
        )

        histogram = diff.histogram()

        error_sum = sum(
            count * (index % 256) ** 2
            for index, count in enumerate(histogram)
        )

        return math.sqrt(
            error_sum / (3 * expected.width * expected.height)
        )


def run_tests(executable, data_dir):
    failed = 0

    with tempfile.TemporaryDirectory(
        prefix="image_processor_tests_"
    ) as temp_dir:

        for name, input_name, args, reference_name, eps in TEST_CASES:
            input_path = data_dir / input_name
            expected_path = data_dir / reference_name
            output_path = Path(temp_dir) / f"{name}.bmp"

            try:
                if not input_path.is_file():
                    raise FileNotFoundError(
                        f"Input image not found: {input_path}"
                    )

                if eps is None:
                    if reference_name == "FileError":
                        output_path = Path(temp_dir) / "missing_directory" / f"{name}.bmp"
                    result = subprocess.run(
                        [str(executable), str(input_path), str(output_path), *args],
                        capture_output=True, text=True, timeout=180,
                    )
                    message = result.stderr.strip()
                    prefix = reference_name + ":"
                    if (result.returncode != 1 or not message.startswith(prefix)
                            or not message[len(prefix):].strip()):
                        raise ValueError(
                            f"Expected {reference_name} and exit=1, "
                            f"got exit={result.returncode}, stderr={message!r}"
                        )
                    print(f"OK   [{name}] {reference_name}")
                    continue

                if not expected_path.is_file():
                    raise FileNotFoundError(
                        f"Reference image not found: {expected_path}"
                    )

                subprocess.run(
                    [
                        str(executable),
                        str(input_path),
                        str(output_path),
                        *args,
                    ],
                    check=True,
                    timeout=180,
                )

                distance = calc_images_distance(
                    expected_path,
                    output_path,
                )

                if distance > eps:
                    print(
                        f"FAIL [{name}] "
                        f"RMS={distance:.4f}, allowed={eps}"
                    )
                    failed += 1
                else:
                    print(f"OK   [{name}] RMS={distance:.4f}")

            except (
                OSError,
                subprocess.SubprocessError,
                ValueError,
            ) as error:
                print(f"FAIL [{name}] {error}")
                failed += 1

    print(f"Passed: {len(TEST_CASES) - failed}/{len(TEST_CASES)}")

    return failed == 0


def main():
    if len(sys.argv) != 2:
        print(
            "Usage: python3 test_image_processor.py "
            "/path/to/image_processor"
        )
        return 2

    executable = Path(sys.argv[1]).expanduser().resolve()
    data_dir = Path(__file__).resolve().parent / "data"

    if not executable.is_file():
        print(f"Executable not found: {executable}")
        return 2

    print(f"Executable: {executable}")
    print(f"Test data: {data_dir}")

    return 0 if run_tests(executable, data_dir) else 1


if __name__ == "__main__":
    sys.exit(main())
