import { defineConfig } from 'vite';

// GitHub Pages serves this project from /Journey-Game-/, not the domain root.
export default defineConfig({
  base: process.env.GITHUB_PAGES === 'true' ? '/Journey-Game-/' : '/',
});
