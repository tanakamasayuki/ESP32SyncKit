# Changelog / 変更履歴

## Unreleased
- (EN) Updated release scripts
- (JA) リリーススクリプト更新
- (EN) Added changelog
- (JA) チェンジログ追加
- (EN) Release workflow now rebuilds the release branch and tags it so rewritten sketch.yaml files are part of the tagged release contents
- (JA) リリースワークフローでreleaseブランチを作り直し、書き換え済みsketch.yamlをタグの内容に含めるように変更
- (EN) Queue<T>: added `count()` (ISR-safe) and `clear()` (task-only) helpers
- (JA) Queue<T> に `count()`（ISR可）と `clear()`（タスク専用）を追加
- (EN) Notify counter mode: `take()` now decrements by 1 (leaves remaining); added `takeAll()` to drain and get the count
- (JA) Notify カウンタモード: `take()` は1件ずつ減算し残カウントを保持。まとめ取りの `takeAll()` を追加
