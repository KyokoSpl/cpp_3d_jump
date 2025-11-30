#!/bin/bash
#
# Build script for C++ 3D Jump Game
# Supports: Arch Linux (.pkg.tar.zst), Debian (.deb), RPM (.rpm), Windows (.exe)
#

set -e

PROJECT_NAME="cpp_3d_jump"
VERSION="1.0.0"
PROJECT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BUILD_DIR="$PROJECT_DIR/build"
PACKAGE_DIR="$PROJECT_DIR/packages"

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

print_header() {
    echo -e "${BLUE}======================================${NC}"
    echo -e "${BLUE}$1${NC}"
    echo -e "${BLUE}======================================${NC}"
}

print_success() {
    echo -e "${GREEN}✓ $1${NC}"
}

print_warning() {
    echo -e "${YELLOW}⚠ $1${NC}"
}

print_error() {
    echo -e "${RED}✗ $1${NC}"
}

show_help() {
    echo "Usage: $0 [OPTIONS]"
    echo ""
    echo "Build packaging script for C++ 3D Jump Game"
    echo ""
    echo "Options:"
    echo "  --all           Build all packages (Linux only)"
    echo "  --arch          Build Arch Linux package (.pkg.tar.zst)"
    echo "  --deb           Build Debian package (.deb)"
    echo "  --rpm           Build RPM package (.rpm)"
    echo "  --windows       Cross-compile Windows executable (.exe)"
    echo "  --appimage      Build AppImage (portable Linux)"
    echo "  --clean         Clean build directories"
    echo "  --help          Show this help message"
    echo ""
    echo "Examples:"
    echo "  $0 --deb        # Build only Debian package"
    echo "  $0 --all        # Build all Linux packages"
    echo "  $0 --windows    # Cross-compile for Windows"
}

check_dependencies() {
    local missing=()
    
    # Check common dependencies
    command -v cmake >/dev/null 2>&1 || missing+=("cmake")
    command -v make >/dev/null 2>&1 || missing+=("make")
    command -v gcc >/dev/null 2>&1 || missing+=("gcc")
    
    if [ ${#missing[@]} -ne 0 ]; then
        print_error "Missing dependencies: ${missing[*]}"
        echo "Please install them first."
        exit 1
    fi
}

check_arch_deps() {
    command -v makepkg >/dev/null 2>&1 || {
        print_error "makepkg not found. Are you on Arch Linux?"
        return 1
    }
}

check_deb_deps() {
    command -v dpkg-deb >/dev/null 2>&1 || {
        print_warning "dpkg-deb not found. Installing dpkg..."
        if command -v apt-get >/dev/null 2>&1; then
            sudo apt-get install -y dpkg
        else
            print_error "Cannot install dpkg. Please install it manually."
            return 1
        fi
    }
}

check_rpm_deps() {
    command -v rpmbuild >/dev/null 2>&1 || {
        print_warning "rpmbuild not found."
        if command -v dnf >/dev/null 2>&1; then
            sudo dnf install -y rpm-build
        elif command -v apt-get >/dev/null 2>&1; then
            sudo apt-get install -y rpm
        elif command -v pacman >/dev/null 2>&1; then
            print_error "On Arch Linux, install rpm-tools from AUR:"
            echo "  yay -S rpm-tools"
            echo "  # or"
            echo "  paru -S rpm-tools"
            return 1
        else
            print_error "Cannot install rpm-build. Please install it manually."
            return 1
        fi
    }
}

check_windows_deps() {
    if ! command -v x86_64-w64-mingw32-gcc >/dev/null 2>&1; then
        print_error "MinGW-w64 not found."
        echo "Install with:"
        echo "  Arch:   sudo pacman -S mingw-w64-gcc"
        echo "  Debian: sudo apt install mingw-w64"
        echo "  Fedora: sudo dnf install mingw64-gcc-c++"
        return 1
    fi
}

clean_build() {
    print_header "Cleaning build directories"
    rm -rf "$BUILD_DIR"
    rm -rf "$PACKAGE_DIR"
    rm -rf "$PROJECT_DIR/build-windows"
    rm -rf "$PROJECT_DIR/pkg"
    rm -rf "$PROJECT_DIR/src"/*.pkg.tar.*
    rm -rf "$PROJECT_DIR"/*.pkg.tar.*
    print_success "Clean complete"
}

build_native() {
    print_header "Building native binary"
    mkdir -p "$BUILD_DIR"
    cd "$BUILD_DIR"
    cmake .. -DCMAKE_BUILD_TYPE=Release
    make -j$(nproc)
    print_success "Native build complete: $BUILD_DIR/$PROJECT_NAME"
}

build_arch() {
    print_header "Building Arch Linux Package"
    check_arch_deps || return 1
    
    mkdir -p "$PACKAGE_DIR"
    
    # Create PKGBUILD
    cat > "$PACKAGE_DIR/PKGBUILD" << 'PKGBUILD_EOF'
# Maintainer: Kyoko Kiese 
pkgname=cpp_3d_jump
pkgver=1.0.0
pkgrel=1
pkgdesc="A 3D obstacle course jump game"
arch=('x86_64')
url="https://github.com/KyokoSpl/cpp_3d_jump"
license=('MIT')
depends=('glfw' 'freetype2' 'libgl')
makedepends=('cmake' 'gcc')

build() {
    cd "$startdir/.."
    cmake -B build-arch \
        -DCMAKE_BUILD_TYPE=Release \
        -DCMAKE_INSTALL_PREFIX=/usr
    cmake --build build-arch -j$(nproc)
}

package() {
    cd "$startdir/.."
    
    # Install binary
    install -Dm755 "build-arch/cpp_3d_jump" "$pkgdir/usr/bin/cpp_3d_jump"
    
    # Install assets
    install -dm755 "$pkgdir/usr/share/cpp_3d_jump"
    cp -r asset "$pkgdir/usr/share/cpp_3d_jump/"
    
    # Install desktop entry
    install -Dm644 /dev/stdin "$pkgdir/usr/share/applications/cpp_3d_jump.desktop" << EOF
[Desktop Entry]
Name=C++ 3D Jump
Comment=A 3D obstacle course jump game
Exec=cpp_3d_jump
Terminal=false
Type=Application
Categories=Game;
EOF
}
PKGBUILD_EOF

    cd "$PACKAGE_DIR"
    makepkg -sf --noconfirm
    
    # Move package to packages dir
    mv *.pkg.tar.* "$PACKAGE_DIR/" 2>/dev/null || true
    
    print_success "Arch package built: $PACKAGE_DIR/*.pkg.tar.zst"
    echo "Install with: sudo pacman -U $PACKAGE_DIR/*.pkg.tar.zst"
}

build_deb() {
    print_header "Building Debian Package"
    check_deb_deps || return 1
    
    mkdir -p "$PACKAGE_DIR"
    
    # Build first
    build_native
    
    # Debian package names must use lowercase alphanums and hyphens only
    local DEB_PKG_NAME="cpp-3d-jump"
    
    # Create package structure
    local DEB_DIR="$PACKAGE_DIR/deb-build"
    rm -rf "$DEB_DIR"
    mkdir -p "$DEB_DIR/DEBIAN"
    mkdir -p "$DEB_DIR/usr/bin"
    mkdir -p "$DEB_DIR/usr/share/$DEB_PKG_NAME"
    mkdir -p "$DEB_DIR/usr/share/applications"
    
    # Copy files
    cp "$BUILD_DIR/$PROJECT_NAME" "$DEB_DIR/usr/bin/$DEB_PKG_NAME"
    cp -r "$PROJECT_DIR/asset" "$DEB_DIR/usr/share/$DEB_PKG_NAME/"
    
    # Create control file
    cat > "$DEB_DIR/DEBIAN/control" << EOF
Package: $DEB_PKG_NAME
Version: $VERSION
Section: games
Priority: optional
Architecture: amd64
Depends: libglfw3, libfreetype6, libgl1
Maintainer: Your Name <your.email@example.com>
Description: A 3D obstacle course jump game
 A first-person 3D obstacle course game where you jump,
 duck, and navigate through various barriers.
EOF

    # Create desktop entry
    cat > "$DEB_DIR/usr/share/applications/$DEB_PKG_NAME.desktop" << EOF
[Desktop Entry]
Name=C++ 3D Jump
Comment=A 3D obstacle course jump game
Exec=$DEB_PKG_NAME
Terminal=false
Type=Application
Categories=Game;
EOF

    # Set permissions
    chmod 755 "$DEB_DIR/usr/bin/$DEB_PKG_NAME"
    
    # Build .deb
    dpkg-deb --build "$DEB_DIR" "$PACKAGE_DIR/${DEB_PKG_NAME}_${VERSION}_amd64.deb"
    
    print_success "Debian package built: $PACKAGE_DIR/${DEB_PKG_NAME}_${VERSION}_amd64.deb"
    echo "Install with: sudo dpkg -i $PACKAGE_DIR/${DEB_PKG_NAME}_${VERSION}_amd64.deb"
}

build_rpm() {
    print_header "Building RPM Package"
    check_rpm_deps || return 1
    
    mkdir -p "$PACKAGE_DIR"
    
    # Build first
    build_native
    
    # Setup RPM build directories
    local RPM_DIR="$PACKAGE_DIR/rpmbuild"
    mkdir -p "$RPM_DIR"/{BUILD,RPMS,SOURCES,SPECS,SRPMS}
    
    # Create tarball for rpmbuild
    local TARBALL_DIR="$RPM_DIR/SOURCES/${PROJECT_NAME}-${VERSION}"
    mkdir -p "$TARBALL_DIR"
    cp "$BUILD_DIR/$PROJECT_NAME" "$TARBALL_DIR/"
    cp -r "$PROJECT_DIR/asset" "$TARBALL_DIR/"
    
    cd "$RPM_DIR/SOURCES"
    tar -czf "${PROJECT_NAME}-${VERSION}.tar.gz" "${PROJECT_NAME}-${VERSION}"
    rm -rf "$TARBALL_DIR"
    
    # Create spec file
    cat > "$RPM_DIR/SPECS/${PROJECT_NAME}.spec" << EOF
Name:           $PROJECT_NAME
Version:        $VERSION
Release:        1%{?dist}
Summary:        A 3D obstacle course jump game

License:        MIT
URL:            https://github.com/KyokoSpl/cpp_3d_jump
Source0:        %{name}-%{version}.tar.gz

Requires:       glfw freetype mesa-libGL

%description
A first-person 3D obstacle course game where you jump,
duck, and navigate through various barriers.

%prep
%setup -q

%install
mkdir -p %{buildroot}%{_bindir}
mkdir -p %{buildroot}%{_datadir}/%{name}
install -m 755 $PROJECT_NAME %{buildroot}%{_bindir}/
cp -r asset %{buildroot}%{_datadir}/%{name}/

%files
%{_bindir}/$PROJECT_NAME
%{_datadir}/%{name}

%changelog
* $(date "+%a %b %d %Y") Your Name <your.email@example.com> - $VERSION-1
- Initial package
EOF

    # Build RPM
    rpmbuild --define "_topdir $RPM_DIR" -bb "$RPM_DIR/SPECS/${PROJECT_NAME}.spec"
    
    # Copy result
    cp "$RPM_DIR/RPMS/"*/*.rpm "$PACKAGE_DIR/" 2>/dev/null || true
    
    print_success "RPM package built in: $PACKAGE_DIR/"
    echo "Install with: sudo dnf install $PACKAGE_DIR/*.rpm"
}

build_windows() {
    print_header "Cross-compiling for Windows"
    check_windows_deps || return 1
    
    mkdir -p "$PACKAGE_DIR"
    
    local WIN_BUILD_DIR="$PROJECT_DIR/build-windows"
    local WIN_OUT_DIR="$PACKAGE_DIR/windows"
    
    mkdir -p "$WIN_BUILD_DIR"
    mkdir -p "$WIN_OUT_DIR"
    
    # Create toolchain file
    cat > "$WIN_BUILD_DIR/toolchain-mingw.cmake" << 'EOF'
set(CMAKE_SYSTEM_NAME Windows)
set(CMAKE_SYSTEM_PROCESSOR x86_64)

set(CMAKE_C_COMPILER x86_64-w64-mingw32-gcc)
set(CMAKE_CXX_COMPILER x86_64-w64-mingw32-g++)
set(CMAKE_RC_COMPILER x86_64-w64-mingw32-windres)

set(CMAKE_FIND_ROOT_PATH /usr/x86_64-w64-mingw32)
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)

# Static linking for easier distribution
set(CMAKE_EXE_LINKER_FLAGS "-static-libgcc -static-libstdc++")
EOF

    print_warning "Windows cross-compilation requires Windows builds of:"
    echo "  - GLFW3"
    echo "  - FreeType"
    echo ""
    echo "You can get prebuilt binaries from:"
    echo "  - GLFW: https://www.glfw.org/download.html"
    echo "  - FreeType: https://github.com/ubawurinna/freetype-windows-binaries"
    echo ""
    echo "Alternative: Use MSYS2 on Windows or WSL2"
    echo ""
    
    # Try to build (will likely fail without deps, but shows the process)
    cd "$WIN_BUILD_DIR"
    
    if cmake "$PROJECT_DIR" \
        -DCMAKE_TOOLCHAIN_FILE="$WIN_BUILD_DIR/toolchain-mingw.cmake" \
        -DCMAKE_BUILD_TYPE=Release 2>/dev/null; then
        
        if make -j$(nproc) 2>/dev/null; then
            cp "$WIN_BUILD_DIR/$PROJECT_NAME.exe" "$WIN_OUT_DIR/"
            cp -r "$PROJECT_DIR/asset" "$WIN_OUT_DIR/"
            
            print_success "Windows build complete: $WIN_OUT_DIR/"
            
            # Create a simple batch launcher
            cat > "$WIN_OUT_DIR/run_game.bat" << 'BATCH'
@echo off
cd /d "%~dp0"
cpp_3d_jump.exe
pause
BATCH
            return 0
        fi
    fi
    
    print_warning "Cross-compilation setup created but build may require additional setup."
    echo "Toolchain file: $WIN_BUILD_DIR/toolchain-mingw.cmake"
    echo ""
    echo "For easier Windows builds, consider:"
    echo "  1. Using GitHub Actions with Windows runner"
    echo "  2. Building natively on Windows with MSYS2/vcpkg"
    echo "  3. Using Docker with a Windows cross-compile image"
}

build_appimage() {
    print_header "Building AppImage"
    
    # Check for appimagetool
    if ! command -v appimagetool >/dev/null 2>&1; then
        print_warning "appimagetool not found. Downloading..."
        mkdir -p "$PACKAGE_DIR/tools"
        wget -q "https://github.com/AppImage/AppImageKit/releases/download/continuous/appimagetool-x86_64.AppImage" \
            -O "$PACKAGE_DIR/tools/appimagetool"
        chmod +x "$PACKAGE_DIR/tools/appimagetool"
        APPIMAGETOOL="$PACKAGE_DIR/tools/appimagetool"
    else
        APPIMAGETOOL="appimagetool"
    fi
    
    mkdir -p "$PACKAGE_DIR"
    
    # Build first
    build_native
    
    # Create AppDir structure
    local APPDIR="$PACKAGE_DIR/Cpp3DJump.AppDir"
    rm -rf "$APPDIR"
    mkdir -p "$APPDIR/usr/bin"
    mkdir -p "$APPDIR/usr/share/cpp_3d_jump"
    mkdir -p "$APPDIR/usr/share/icons/hicolor/256x256/apps"
    
    # Copy files
    cp "$BUILD_DIR/$PROJECT_NAME" "$APPDIR/usr/bin/"
    cp -r "$PROJECT_DIR/asset" "$APPDIR/usr/share/cpp_3d_jump/"
    
    # Create desktop entry
    cat > "$APPDIR/cpp_3d_jump.desktop" << EOF
[Desktop Entry]
Name=C++ 3D Jump
Comment=A 3D obstacle course jump game
Exec=cpp_3d_jump
Icon=cpp_3d_jump
Terminal=false
Type=Application
Categories=Game;
EOF

    # Create AppRun
    cat > "$APPDIR/AppRun" << 'EOF'
#!/bin/bash
SELF=$(readlink -f "$0")
HERE=${SELF%/*}
export PATH="${HERE}/usr/bin:${PATH}"
export LD_LIBRARY_PATH="${HERE}/usr/lib:${LD_LIBRARY_PATH}"
cd "${HERE}/usr/share/cpp_3d_jump"
exec "${HERE}/usr/bin/cpp_3d_jump" "$@"
EOF
    chmod +x "$APPDIR/AppRun"
    
    # Create a simple icon (placeholder)
    # In a real scenario, you'd have a proper icon
    cat > "$APPDIR/cpp_3d_jump.svg" << 'EOF'
<svg xmlns="http://www.w3.org/2000/svg" width="256" height="256">
  <rect width="256" height="256" fill="#4a90d9"/>
  <text x="128" y="140" font-size="48" text-anchor="middle" fill="white">3D</text>
</svg>
EOF
    cp "$APPDIR/cpp_3d_jump.svg" "$APPDIR/usr/share/icons/hicolor/256x256/apps/"
    
    # Build AppImage
    cd "$PACKAGE_DIR"
    ARCH=x86_64 "$APPIMAGETOOL" "$APPDIR" "${PROJECT_NAME}-${VERSION}-x86_64.AppImage" 2>/dev/null || {
        print_warning "AppImage creation may require --appimage-extract-and-run on some systems"
        ARCH=x86_64 "$APPIMAGETOOL" --appimage-extract-and-run "$APPDIR" "${PROJECT_NAME}-${VERSION}-x86_64.AppImage" 2>/dev/null || true
    }
    
    if [ -f "$PACKAGE_DIR/${PROJECT_NAME}-${VERSION}-x86_64.AppImage" ]; then
        print_success "AppImage built: $PACKAGE_DIR/${PROJECT_NAME}-${VERSION}-x86_64.AppImage"
        echo "Run with: chmod +x *.AppImage && ./*.AppImage"
    else
        print_error "AppImage creation failed"
    fi
}

build_all() {
    print_header "Building All Packages"
    
    local failed=()
    
    build_deb || failed+=("deb")
    build_rpm || failed+=("rpm")
    build_appimage || failed+=("appimage")
    
    # Only try Arch on Arch-based systems
    if command -v pacman >/dev/null 2>&1; then
        build_arch || failed+=("arch")
    else
        print_warning "Skipping Arch package (not on Arch-based system)"
    fi
    
    echo ""
    print_header "Build Summary"
    echo "Packages directory: $PACKAGE_DIR"
    ls -la "$PACKAGE_DIR"/*.deb "$PACKAGE_DIR"/*.rpm "$PACKAGE_DIR"/*.AppImage 2>/dev/null || true
    
    if [ ${#failed[@]} -ne 0 ]; then
        print_warning "Some builds failed: ${failed[*]}"
    else
        print_success "All builds completed successfully!"
    fi
}

# Main
main() {
    cd "$PROJECT_DIR"
    check_dependencies
    
    if [ $# -eq 0 ]; then
        show_help
        exit 0
    fi
    
    case "$1" in
        --all)
            build_all
            ;;
        --arch)
            build_arch
            ;;
        --deb)
            build_deb
            ;;
        --rpm)
            build_rpm
            ;;
        --windows)
            build_windows
            ;;
        --appimage)
            build_appimage
            ;;
        --clean)
            clean_build
            ;;
        --help|-h)
            show_help
            ;;
        *)
            print_error "Unknown option: $1"
            show_help
            exit 1
            ;;
    esac
}

main "$@"
