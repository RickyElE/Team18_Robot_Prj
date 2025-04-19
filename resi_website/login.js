document.addEventListener('DOMContentLoaded', () => {
    // Hide loading overlay after page loads
    setTimeout(() => {
        document.getElementById('loadingOverlay').classList.add('hide');
    }, 1500);

    // Handle form submission
    document.getElementById('loginForm').addEventListener('submit', (e) => {
        e.preventDefault();
        const username = document.getElementById('username').value;
        const password = document.getElementById('password').value;

        // Show loading overlay
        document.getElementById('loadingOverlay').classList.remove('hide');

        // Simulate login process (you can replace this with actual authentication)
        if (username === "robot" && password === "robot") {
            setTimeout(() => {
                // Hide loading overlay
                document.getElementById('loadingOverlay').classList.add('hide');
                
                // Show welcome message
                const welcomeOverlay = document.getElementById('welcomeOverlay');
                const welcomeUser = document.getElementById('welcomeUser');
                welcomeUser.textContent = username;
                welcomeOverlay.classList.add('show');

                // Store login status and username
                localStorage.setItem('isLoggedIn', 'true');
                localStorage.setItem('username', username);

                // Redirect after welcome message
                setTimeout(() => {
                    window.location.href = 'website.html';
                }, 2000);
            }, 1500);
        } else {
            setTimeout(() => {
                alert('Invalid username or password. Please try again.');
                document.getElementById('loadingOverlay').classList.add('hide');
            }, 1500);
        }
    });
});