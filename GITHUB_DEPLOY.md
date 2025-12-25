# Deploying to GitHub

This guide explains how to upload the Consumption Counter Module to GitHub.

## Prerequisites

- Git installed on your system
- GitHub account
- Repository created at https://github.com/Kavis1/consumption-counter

## Step 1: Initialize Git Repository

```bash
# Navigate to project directory
cd /path/to/consumption-counter

# Initialize git repository
git init

# Add all files
git add .

# Create initial commit
git commit -m "Initial commit: Consumption Counter Module v1.0.0

- Anonymous consumption tracking module for vending machines
- C99 compliant, multi-platform support (STM32, NXP, Linux)
- MIT licensed, open source
- Complete documentation and examples"

# Set remote repository
git remote add origin https://github.com/Kavis1/consumption-counter.git

# Push to GitHub
git push -u origin main
```

## Step 2: Verify Repository Structure

After pushing, verify that all files are uploaded correctly:

### Required Files
- ✅ `README.md` - Main documentation
- ✅ `LICENSE` - MIT license
- ✅ `CONTRIBUTING.md` - Contribution guidelines
- ✅ `CHANGELOG.md` - Version history
- ✅ `.gitignore` - Git ignore rules

### Source Code
- ✅ `include/` - Header files
  - `consumption.h`
  - `consumption_platform.h`
  - `consumption_network.h`
- ✅ `src/` - Implementation files
  - `consumption.c`
  - Platform implementations
- ✅ `examples/` - Usage examples
- ✅ `tests/` - Unit tests
- ✅ `docs/` - Documentation

### GitHub Features
- ✅ `.github/ISSUE_TEMPLATES/` - Issue templates
- ✅ GitHub badges in README
- ✅ Proper repository description

## Step 3: Configure Repository Settings

### Repository Description
```
🍶 Anonymous consumption tracking module for vending machines - collect beverage sales statistics without affecting core functionality.
```

### Topics/Tags
```
c, embedded, vending-machine, consumption-tracking, iot, mit-license, multi-platform, stm32, nxp, linux
```

### Repository Settings
- **Repository name**: `consumption-counter`
- **Description**: As above
- **Visibility**: Public
- **Features**:
  - ✅ Issues
  - ✅ Projects (optional)
  - ✅ Wiki (optional)
  - ✅ Discussions (optional)

## Step 4: Enable GitHub Features

### Issues Templates
The repository includes issue templates in `.github/ISSUE_TEMPLATES/`:
- 🐛 Bug reports
- ✨ Feature requests
- 📚 Documentation issues
- 🔧 Platform support requests
- 🆘 Support requests

### Branch Protection (Optional)
Consider setting up branch protection for `main`:
- Require pull request reviews
- Require status checks
- Include administrators

## Step 5: Post-Deployment Tasks

### Create Release
```bash
# Create and push tag
git tag -a v1.0.0 -m "Release v1.0.0"
git push origin v1.0.0

# Create release on GitHub
# Go to https://github.com/Kavis1/consumption-counter/releases
# Click "Create a new release"
# Tag: v1.0.0
# Title: Consumption Counter Module v1.0.0
# Description: Copy from CHANGELOG.md
```

### Documentation Links
Update any external documentation to point to:
- **Repository**: https://github.com/Kavis1/consumption-counter
- **Issues**: https://github.com/Kavis1/consumption-counter/issues
- **Releases**: https://github.com/Kavis1/consumption-counter/releases

## Step 6: Community Setup

### README Badges
The repository includes badges that should work automatically:
- GitHub repository link
- License information
- Build status (if CI is set up)
- Issues link

### Contributing
The `CONTRIBUTING.md` file is ready and includes:
- Development setup instructions
- Coding standards
- Testing guidelines
- Pull request templates

## Troubleshooting

### Common Issues

**Files not uploading:**
```bash
# Check git status
git status

# Add any missed files
git add .

# Check file sizes (GitHub has limits)
ls -lh
```

**Repository not found:**
- Verify repository URL
- Check repository name spelling
- Ensure repository is public

**Permission denied:**
- Verify GitHub credentials
- Check SSH key setup (if using SSH)

**Large files:**
- Check for build artifacts in repository
- Use `.gitignore` to exclude them
- Consider using Git LFS for large files if needed

## Verification Checklist

- [ ] Repository created on GitHub
- [ ] All files uploaded successfully
- [ ] README renders correctly
- [ ] Badges are working
- [ ] License file is present
- [ ] Issue templates are active
- [ ] Repository has description and topics
- [ ] Initial release created
- [ ] Documentation links are correct

## Next Steps

After deployment:

1. **Announce the project** in relevant communities
2. **Monitor issues** and respond promptly
3. **Plan next features** based on community feedback
4. **Set up CI/CD** for automated testing
5. **Consider documentation website** (GitHub Pages)

---

**Repository URL**: https://github.com/Kavis1/consumption-counter
**Documentation**: https://github.com/Kavis1/consumption-counter#readme
